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
    using namespace std::chrono_literals;

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

    void http_ok_callback(unet::net_core &nc, void *)
    {
        char buffer[1024] = {};
        (void)nc.recv_data(buffer, sizeof(buffer) - 1, 1000);

        const std::string body = "ok";
        const std::string header = unet::http::get_http_result_header(
            "200 OK", "text/plain; charset=UTF-8", body.size());
        const std::string response = header + body;
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
        const std::string body = "standby";
        const std::string header = unet::http::get_http_result_header(
            "200 OK", "text/plain; charset=UTF-8", body.size());
        const std::string response = header + body;
        (void)sv.send_data(response, response.size());
        (void)sv.close_s();
    }

    void test_http_helpers()
    {
        const std::string req = unet::http::get_http_request_header("GET", "/a", "localhost", "cppnet-test");
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

    void test_server_client_tcp()
    {
        constexpr int port = 18100;
        unet::Server server(port, http_ok_callback, unet::TCP_c, "server.crt", "server.key", false);
        expect(server.listen_p(false) == unet::success, "server listen_p(TCP) failed");
        std::this_thread::sleep_for(150ms);

        unet::Client client;
        (void)client.connect_s("::1", unet::TCP_c, port);
        const std::string request = unet::http::get_http_request_header("GET", "/", "localhost", "cppnet-test");
        (void)client.send_data(request, request.size());
        const std::string response = client.recv_all(3000);
        (void)client.close_s();
        (void)server.stop();

        expect(response.starts_with("HTTP/1.1 200 OK"), "TCP response status mismatch");
        expect(unet::http::extract_http_body(response) == "ok", "TCP response body mismatch");
    }

    void test_server_client_ssl()
    {
        constexpr int port = 18101;
        unet::Server server(port, http_ok_callback, unet::SSL_c, "server.crt", "server.key", false);
        expect(server.listen_p(false) == unet::success, "server listen_p(SSL) failed");
        std::this_thread::sleep_for(150ms);

        unet::Client client;
        (void)client.connect_s("::1", unet::SSL_c, port);
        const std::string request = unet::http::get_http_request_header("GET", "/", "localhost", "cppnet-test");
        (void)client.send_data(request, request.size());
        const std::string response = client.recv_all(3000);
        (void)client.close_s();
        (void)server.stop();

        expect(response.starts_with("HTTP/1.1 200 OK"), "SSL response status mismatch");
        expect(unet::http::extract_http_body(response) == "ok", "SSL response body mismatch");
    }

    void test_standby_client_modes()
    {
        constexpr int tcp_port = 18110;
        {
            unet::Server server(tcp_port, http_ok_callback, unet::TCP_c, "server.crt", "server.key", false);
            expect(server.listen_p(false) == unet::success, "server listen_p for standby TCP failed");
            std::this_thread::sleep_for(150ms);

            unet::Standby sb(tcp_port, unet::TCP_c);
            (void)sb.set(tcp_port, unet::TCP_c);
            (void)sb.change_type(unet::TCP_c);
            (void)sb.connect_s("::1");
            const std::string req = unet::http::get_http_request_header("GET", "/", "localhost", "cppnet-test");
            (void)sb.send_data(req, req.size());
            const std::string response = sb.recv_all(3000);
            (void)sb.close_s();
            (void)server.stop();
            expect(response.starts_with("HTTP/1.1 200 OK"), "standby TCP client failed");
        }

        constexpr int ssl_port = 18111;
        {
            unet::Server server(ssl_port, http_ok_callback, unet::SSL_c, "server.crt", "server.key", false);
            expect(server.listen_p(false) == unet::success, "server listen_p for standby SSL failed");
            std::this_thread::sleep_for(150ms);

            unet::Standby sb(ssl_port, unet::SSL_c);
            (void)sb.set(ssl_port, unet::SSL_c);
            (void)sb.change_type(unet::SSL_c);
            (void)sb.connect_s("::1");
            const std::string req = unet::http::get_http_request_header("GET", "/", "localhost", "cppnet-test");
            (void)sb.send_data(req, req.size());
            const std::string response = sb.recv_all(3000);
            (void)sb.close_s();
            (void)server.stop();
            expect(response.starts_with("HTTP/1.1 200 OK"), "standby SSL client failed");
        }
    }

    void test_standby_server_modes()
    {
        constexpr int tcp_port = 18120;
        {
            std::thread server_thread(standby_server_once, unet::TCP_c, tcp_port);
            std::this_thread::sleep_for(150ms);

            unet::Standby client(tcp_port, unet::TCP_c);
            (void)client.set(tcp_port, unet::TCP_c);
            (void)client.connect_s("::1");
            const std::string req = unet::http::get_http_request_header("GET", "/", "localhost", "cppnet-test");
            (void)client.send_data(req, req.size());
            const std::string response = client.recv_all(3000);
            (void)client.close_s();
            server_thread.join();

            expect(response.starts_with("HTTP/1.1 200 OK"), "standby TCP server failed");
            expect(unet::http::extract_http_body(response) == "standby", "standby TCP body mismatch");
        }

        constexpr int ssl_port = 18121;
        {
            std::thread server_thread(standby_server_once, unet::SSL_c, ssl_port);
            std::this_thread::sleep_for(150ms);

            unet::Standby client(ssl_port, unet::SSL_c);
            (void)client.set(ssl_port, unet::SSL_c);
            (void)client.connect_s("::1");
            const std::string req = unet::http::get_http_request_header("GET", "/", "localhost", "cppnet-test");
            (void)client.send_data(req, req.size());
            const std::string response = client.recv_all(3000);
            (void)client.close_s();
            server_thread.join();

            expect(response.starts_with("HTTP/1.1 200 OK"), "standby SSL server failed");
            expect(unet::http::extract_http_body(response) == "standby", "standby SSL body mismatch");
        }
    }

    void test_udp_features()
    {
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
        {"server/client tcp", test_server_client_tcp},
        {"server/client ssl", test_server_client_ssl},
        {"standby client modes", test_standby_client_modes},
        {"standby server modes", test_standby_server_modes},
        {"udp features", test_udp_features},
    };

    int failed = 0;
    for (const auto &test : tests)
    {
        try
        {
            test.fn();
            std::cout << "[PASS] " << test.name << "\n";
        }
        catch (const std::exception &e)
        {
            failed++;
            std::cerr << "[FAIL] " << test.name << " : " << e.what() << "\n";
        }
        catch (...)
        {
            failed++;
            std::cerr << "[FAIL] " << test.name << " : unknown error\n";
        }
    }

    unet::netcpp_stop();

    if (failed == 0)
    {
        std::cout << "All tests passed (" << tests.size() << "/" << tests.size() << ")\n";
        return 0;
    }

    std::cerr << failed << " test(s) failed\n";
    return 1;
}
