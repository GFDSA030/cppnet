#include <unet.h>

#include <chrono>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace
{
    namespace console
    {
        constexpr const char *reset = "\033[0m\033[39m\033[49m";
        namespace colors
        {
            constexpr const char *red = "\033[31m";
            constexpr const char *green = "\033[32m";
            constexpr const char *yellow = "\033[33m";
            constexpr const char *cyan = "\033[36m";
            constexpr const char *white = "\033[37m";
        }
    } // namespace console

    using namespace std::chrono_literals;
    constexpr const char *kLoopback = "::1";
    constexpr const char *kHost = "localhost";
    constexpr const char *kUserAgent = "cppnet-test";
    constexpr const char *kCertFile = "server.crt";
    constexpr const char *kKeyFile = "server.key";
    constexpr auto kServerStartDelay = 150ms;
    constexpr int kTimeoutMs = 3000;

    struct TestFailure : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    void expect(bool cond, const std::string &message)
    {
        if (!cond)
            throw TestFailure(message);
    }

    struct TestCase
    {
        std::string name;
        std::function<void()> fn;
    };

    struct NetBaseProbe : public unet::net_base
    {
        int expose_set_type(unet::sock_type type_) noexcept { return set_type(type_); }
        static size_t expose_base_no() noexcept { return get_base_no(); }
        static size_t expose_base_len() noexcept { return get_base_len(); }
        size_t expose_this_no() const noexcept { return get_this_no(); }
    };

    struct UdpCoreProbe : public unet::udp_core
    {
        using unet::udp_core::udp_core;
        int expose_send_m(const unet::IPaddress *addr, const char *buf, int len) const noexcept { return send_m(addr, buf, len); }
        int expose_recv_m(unet::IPaddress *addr, char *buf, int len, int32_t timeout = -1) const noexcept { return recv_m(addr, buf, len, timeout); }
    };

    std::string make_get_request(const std::string &path = "/")
    {
        return unet::http::get_http_request_header("GET", path, kHost, kUserAgent);
    }

    std::string make_http_ok_response(const std::string &body)
    {
        const std::string header = unet::http::get_http_result_header(
            "200 OK", "text/plain; charset=UTF-8", body.size());
        return header + body;
    }

    void start_server(unet::Server &server, const std::string &failure_message)
    {
        expect(server.listen_p(false) == unet::success, failure_message);
        std::this_thread::sleep_for(kServerStartDelay);
    }

    void expect_http_ok(const std::string &response, const std::string &context)
    {
        expect(response.starts_with("HTTP/1.1 200 OK"), context + " status mismatch");
    }

    void expect_http_body(const std::string &response, const std::string &expected_body, const std::string &context)
    {
        expect(unet::http::extract_http_body(response) == expected_body, context + " body mismatch");
    }

    std::string request_with_client(unet::sock_type type, int port, const std::string &request = make_get_request())
    {
        unet::Client client;
        (void)client.connect_s(kLoopback, type, port);
        (void)client.send_data(request, request.size());
        const std::string response = client.recv_all(kTimeoutMs);
        (void)client.close_s();
        return response;
    }

    std::string request_with_standby(unet::sock_type type, int port, const std::string &request = make_get_request())
    {
        unet::Standby sb(port, type);
        (void)sb.set(port, type);
        (void)sb.connect_s(kLoopback);
        (void)sb.send_data(request, request.size());
        const std::string response = sb.recv_all(kTimeoutMs);
        (void)sb.close_s();
        return response;
    }

    void http_ok_callback(unet::net_core &nc, void *)
    {
        char buffer[1024] = {};
        (void)nc.recv_data(buffer, sizeof(buffer) - 1, 1000);

        const std::string response = make_http_ok_response("ok");
        (void)nc.send_data(response, response.size());
        (void)nc.close_s();
    }

    void standby_server_once(unet::sock_type type, int port)
    {
        unet::Standby sv(port, type);
        (void)sv.set(port, type);
        if (sv.accept_s("server.crt", "server.key") != unet::success)
            return;

        std::string request;
        (void)sv.recv_data(request, 4096, 2000);
        const std::string response = make_http_ok_response("standby");
        (void)sv.send_data(response, response.size());
        (void)sv.close_s();
    }

    void test_http_helpers()
    {
        const std::string req = make_get_request("/a");
        expect(req.starts_with("GET /a HTTP/1.1\r\n"), "request line mismatch");
        expect(req.find("Host: localhost\r\n") != std::string::npos, "host header missing");
        expect(req.ends_with("\r\n\r\n"), "request header should end with CRLF CRLF");

        const std::string res = unet::http::get_http_result_header(
            "200 OK", "text/plain", 5, "X-Test: 1\r\n");
        expect(res.find("HTTP/1.1 200 OK\r\n") == 0, "status line mismatch");
        expect(res.find("Content-Length: 5\r\n") != std::string::npos, "content-length missing");
        expect(res.find("X-Test: 1\r\n") != std::string::npos, "custom header missing");

        const std::string packet = "HTTP/1.1 200 OK\r\nA: B\r\n\r\nhello";
        expect(unet::http::extract_http_header(packet) == "HTTP/1.1 200 OK\r\nA: B\r\n",
               "extract_http_header mismatch");
        expect(unet::http::extract_http_body(packet) == "hello",
               "extract_http_body mismatch");
    }

    void test_ip_helpers()
    {
        unet::IPaddress ip = {};
        expect(unet::getipaddr("localhost", ip) == unet::success, "getipaddr failed");
        expect(!unet::ip2str(ip).empty(), "ip2str(getipaddr) is empty");

        unet::IPaddress resolved = {};
        expect(unet::getipaddrinfo("localhost", 18001, resolved, unet::TCP_c) == unet::success,
               "getipaddrinfo failed");
        expect(resolved.ss_family == AF_INET || resolved.ss_family == AF_INET6,
               "unexpected address family");
        if (resolved.ss_family == AF_INET)
            expect(ntohs(((sockaddr_in *)&resolved)->sin_port) == 18001, "IPv4 port mismatch");
        if (resolved.ss_family == AF_INET6)
            expect(ntohs(((sockaddr_in6 *)&resolved)->sin6_port) == 18001, "IPv6 port mismatch");
    }

    void test_net_base_surface()
    {
        NetBaseProbe probe;
        expect(NetBaseProbe::expose_base_no() >= 1, "net_base base_no should be >= 1");
        expect(NetBaseProbe::expose_base_len() >= 1, "net_base base_len should be >= 1");
        expect(probe.expose_this_no() >= 1, "net_base this_no should be >= 1");

        auto exercise_type = [&](unet::sock_type t, const char *name)
        {
            expect(probe.expose_set_type(t) == unet::success, std::string("net_base set_type(") + name + ") failed");
            const char one = 'x';
            expect(probe.send_data(&one, 1) == unet::error, std::string("net_base send_data(") + name + ") on offline should fail");
            char raw[8] = {};
            expect(probe.recv_data(raw, sizeof(raw), 1) == unet::error, std::string("net_base recv_data(") + name + ") on offline should fail");
            expect(probe.close_s() == unet::error, std::string("net_base close_s(") + name + ") on offline should fail");
        };

        exercise_type(unet::TCP_c, "TCP");
        exercise_type(unet::CRY_c, "CRY");
#ifdef NETCPP_SSL_AVAILABLE
        exercise_type(unet::SSL_c, "SSL");
#endif
        expect(probe.expose_set_type(unet::unknown) == unet::error, "net_base set_type(unknown) should fail");

        const char one = 'x';
        expect(probe.send_data(&one, 0) == unet::error, "net_base send_data(ptr,len=0) should fail");

        std::string s;
        expect(probe.recv_data(s, 0, 1) == 0, "net_base recv_data(string,len=0) should return 0");
    }

    void test_net_base_recv_overloads()
    {
        constexpr int str_port = 18102;
        {
            unet::Server server(str_port, http_ok_callback, unet::TCP_c, kCertFile, kKeyFile, false);
            start_server(server, "server listen_p for net_base string recv failed");

            unet::Client client;
            (void)client.connect_s(kLoopback, unet::TCP_c, str_port);
            const std::string request = make_get_request("/");
            (void)client.send_data(request);
            std::string partial;
            const int n = client.recv_data(partial, 256, kTimeoutMs);
            (void)client.close_s();
            (void)server.stop();
            expect(n > 0, "net_base recv_data(string) should read bytes");
        }

        constexpr int raw_port = 18103;
        {
            unet::Server server(raw_port, http_ok_callback, unet::TCP_c, kCertFile, kKeyFile, false);
            start_server(server, "server listen_p for net_base raw recv failed");

            unet::Client client;
            (void)client.connect_s(kLoopback, unet::TCP_c, raw_port);
            const std::string request = make_get_request("/");
            (void)client.send_data(request.c_str(), request.size());
            char raw[256] = {};
            const int n = client.recv_data(raw, sizeof(raw), kTimeoutMs);
            (void)client.close_s();
            (void)server.stop();
            expect(n > 0, "net_base recv_data(ptr) should read bytes");
        }
    }

    void test_server_client_tcp()
    {
        constexpr int port = 18100;
        unet::Server server(port, http_ok_callback, unet::TCP_c, kCertFile, kKeyFile, false);
        start_server(server, "server listen_p(TCP) failed");
        const std::string response = request_with_client(unet::TCP_c, port);
        (void)server.stop();

        expect_http_ok(response, "TCP response");
        expect_http_body(response, "ok", "TCP response");
    }

    void test_server_client_ssl()
    {
        constexpr int port = 18101;
        unet::Server server(port, http_ok_callback, unet::SSL_c, kCertFile, kKeyFile, false);
        start_server(server, "server listen_p(SSL) failed");
        const std::string response = request_with_client(unet::SSL_c, port);
        (void)server.stop();

        expect_http_ok(response, "SSL response");
        expect_http_body(response, "ok", "SSL response");
    }

    void test_standby_client_tcp()
    {
        constexpr int tcp_port = 18110;
        unet::Server server(tcp_port, http_ok_callback, unet::TCP_c, kCertFile, kKeyFile, false);
        start_server(server, "server listen_p for standby TCP failed");
        const std::string response = request_with_standby(unet::TCP_c, tcp_port);
        (void)server.stop();
        expect_http_ok(response, "standby TCP client");
    }

    void test_standby_client_ssl()
    {
        constexpr int ssl_port = 18111;
        unet::Server server(ssl_port, http_ok_callback, unet::SSL_c, kCertFile, kKeyFile, false);
        start_server(server, "server listen_p for standby SSL failed");
        const std::string response = request_with_standby(unet::SSL_c, ssl_port);
        (void)server.stop();
        expect_http_ok(response, "standby SSL client");
    }

    void test_standby_server_tcp()
    {
        constexpr int tcp_port = 18120;
        std::thread server_thread(standby_server_once, unet::TCP_c, tcp_port);
        std::this_thread::sleep_for(kServerStartDelay);

        unet::Standby client(tcp_port, unet::TCP_c);
        (void)client.set(tcp_port, unet::TCP_c);
        (void)client.connect_s(kLoopback);
        const std::string req = make_get_request("/");
        (void)client.send_data(req, req.size());
        const std::string response = client.recv_all(kTimeoutMs);
        (void)client.close_s();
        server_thread.join();

        expect_http_ok(response, "standby TCP server");
        expect_http_body(response, "standby", "standby TCP");
    }

    void test_standby_server_ssl()
    {
        constexpr int ssl_port = 18121;
        std::thread server_thread(standby_server_once, unet::SSL_c, ssl_port);
        std::this_thread::sleep_for(kServerStartDelay);

        unet::Standby client(ssl_port, unet::SSL_c);
        (void)client.set(ssl_port, unet::SSL_c);
        (void)client.connect_s(kLoopback);
        const std::string req = make_get_request("/");
        (void)client.send_data(req, req.size());
        const std::string response = client.recv_all(kTimeoutMs);
        (void)client.close_s();
        server_thread.join();

        expect_http_ok(response, "standby SSL server");
        expect_http_body(response, "standby", "standby SSL");
    }

    void test_udp_features()
    {
        // protected send_m/recv_m + parameterized constructor
        {
            UdpCoreProbe udp(18169, 18170);
            unet::IPaddress dst = {};
            expect(unet::getipaddrinfo("::1", 18170, dst, unet::UDP_c) == unet::success,
                   "udp getipaddrinfo for protected call failed");
            const std::string payload = "pm";
            expect(udp.expose_send_m(&dst, payload.c_str(), (int)payload.size()) == (int)payload.size(),
                   "udp send_m failed");

            char out[32] = {};
            unet::IPaddress from = {};
            const int n = udp.expose_recv_m(&from, out, sizeof(out), 2000);
            expect(n == (int)payload.size(), "udp recv_m size mismatch");
            expect(std::string(out, n) == payload, "udp recv_m payload mismatch");
        }

        // timeout path
        {
            unet::udp_core udp;
            expect(udp.set_port(18131, 18130) == unet::success, "udp set_port timeout case failed");
            char buf[64] = {};
            const int ret = udp.recv_data(buf, sizeof(buf), 120);
            expect(ret == 0, "udp timeout should return 0");
        }

        // recv_data(IPaddress*, ...) + echo
        {
            std::thread server_thread([]()
                                      {
                                          unet::udp_core server;
                                          (void)server.set_port(18141, 18140);
                                          char in[128] = {};
                                          unet::IPaddress from = {};
                                          int n = server.recv_data(&from, in, sizeof(in), 2000);
                                          if (n > 0)
                                          {
                                              std::string host = unet::ip2str(from);
                                              (void)server.send_data(host.c_str(), in, n);
                                          } });

            std::this_thread::sleep_for(100ms);
            unet::udp_core client;
            expect(client.set_port(18140, 18141) == unet::success, "udp set_port echo case failed");
            const std::string payload = "udp-echo";
            expect(client.send_data("::1", payload.c_str(), (int)payload.size()) > 0, "udp send failed");

            char out[128] = {};
            int n = client.recv_data(out, sizeof(out), 2000);
            server_thread.join();
            expect(n == (int)payload.size(), "udp echoed size mismatch");
            expect(std::string(out, n) == payload, "udp echoed payload mismatch");
        }

        // recv_all(IPaddress*, ...)
        {
            std::thread sender([]()
                               {
                                   std::this_thread::sleep_for(100ms);
                                   unet::udp_core tx;
                                   (void)tx.set_port(18151, 18150);
                                   const std::string payload = "udp-all";
                                   (void)tx.send_data("::1", payload.c_str(), (int)payload.size()); });

            unet::udp_core rx;
            expect(rx.set_port(18150, 18151) == unet::success, "udp set_port recv_all case failed");
            unet::IPaddress from = {};
            const std::string all = rx.recv_all(&from, 2000);
            sender.join();
            expect(all == "udp-all", "udp recv_all payload mismatch");
            expect(!unet::ip2str(from).empty(), "udp recv_all source address missing");
        }

        // invalid port arguments
        {
            unet::udp_core udp;
            expect(udp.set_port(-1, 18160) == unet::error, "udp invalid TX port should fail");
            expect(udp.set_port(18160, 70000) == unet::error, "udp invalid RX port should fail");
        }
    }
} // namespace

int main()
{
    unet::netcpp_start();

    const std::vector<TestCase> tests = {
        {"http helpers", test_http_helpers},
        {"ip helpers", test_ip_helpers},
        {"net_base surface", test_net_base_surface},
        {"net_base recv overloads", test_net_base_recv_overloads},
        {"server/client tcp", test_server_client_tcp},
        {"server/client ssl", test_server_client_ssl},
        {"standby client tcp", test_standby_client_tcp},
        {"standby client ssl", test_standby_client_ssl},
        {"standby server tcp", test_standby_server_tcp},
        {"standby server ssl", test_standby_server_ssl},
        {"udp features", test_udp_features},
    };

    int failed = 0;
    int passed = 0;
    std::vector<std::string> failed_tests;

    std::cout << console::colors::cyan
              << "==== cppnet test2 begin (" << tests.size() << " tests) ===="
              << console::reset << "\n";

    for (const auto &test : tests)
    {
        std::cout << console::colors::white << "[RUN ] " << test.name << console::reset << "\n";
        try
        {
            test.fn();
            passed++;
            std::cout << console::colors::green << "[PASS] " << test.name << console::reset << "\n";
        }
        catch (const std::exception &e)
        {
            failed++;
            failed_tests.push_back(test.name);
            std::cerr << console::colors::red << "[FAIL] " << test.name << " : " << e.what()
                      << console::reset << "\n";
        }
        catch (...)
        {
            failed++;
            failed_tests.push_back(test.name);
            std::cerr << console::colors::red << "[FAIL] " << test.name << " : unknown error"
                      << console::reset << "\n";
        }
    }

    unet::netcpp_stop();

    std::cout << console::colors::cyan << "==== cppnet test2 summary ====" << console::reset << "\n";
    std::cout << console::colors::green << "Passed: " << passed << console::reset << " / " << tests.size() << "\n";
    if (failed > 0)
        std::cout << console::colors::red << "Failed: " << failed << console::reset << "\n";
    else
        std::cout << console::colors::green << "Failed: 0" << console::reset << "\n";

    if (!failed_tests.empty())
    {
        std::cerr << console::colors::yellow << "Failed test list:" << console::reset << "\n";
        for (const auto &name : failed_tests)
            std::cerr << console::colors::yellow << " - " << name << console::reset << "\n";
    }

    if (failed == 0)
    {
        std::cout << console::colors::green
                  << "All tests passed (" << tests.size() << "/" << tests.size() << ")"
                  << console::reset << "\n";
        return 0;
    }

    std::cerr << console::colors::red << failed << " test(s) failed" << console::reset << "\n";
    return 1;
}
