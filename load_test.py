#!/usr/bin/env python3
import argparse
import http.client
import threading
import time
from dataclasses import dataclass
from typing import Optional
from urllib.parse import urlparse


TARGET_URL = "http://localhost:9090/"
REQUEST_TIMEOUT_SEC = 3.0


@dataclass
class RunConfig:
    total_time_sec: float = 30.0
    max_requests: Optional[int] = None
    max_concurrency: int = 100
    interval_sec: float = 0.0


# Set this to True to use code-side settings instead of CLI options.
USE_CODE_CONFIG = True
CODE_CONFIG = RunConfig()


@dataclass
class WorkerResult:
    sent: int = 0
    ok: int = 0
    failed: int = 0
    bytes_read: int = 0


class RequestBudget:
    def __init__(self, total_requests: Optional[int]) -> None:
        self.total_requests = total_requests
        self._lock = threading.Lock()
        self._sent = 0

    def acquire(self) -> bool:
        if self.total_requests is None:
            return True
        with self._lock:
            if self._sent >= self.total_requests:
                return False
            self._sent += 1
            return True


def validate_config(cfg: RunConfig) -> None:
    if cfg.max_concurrency <= 0:
        raise SystemExit("--max-concurrency must be > 0")
    if cfg.interval_sec < 0:
        raise SystemExit("--interval must be >= 0")
    if cfg.total_time_sec <= 0 and (cfg.max_requests is None or cfg.max_requests <= 0):
        raise SystemExit("Set total_time_sec > 0 or max_requests > 0")
    if cfg.max_requests is not None and cfg.max_requests <= 0:
        raise SystemExit("--max-requests must be > 0")


def parse_args() -> RunConfig:
    parser = argparse.ArgumentParser(description="Simple HTTP load test")
    parser.add_argument(
        "--total-time",
        "--trial-time",
        dest="total_time_sec",
        type=float,
        default=10.0,
        help="Total test time in seconds. If > 0, this is prioritized.",
    )
    parser.add_argument(
        "--max-requests",
        type=int,
        default=None,
        help="Maximum request count. Used when total-time <= 0.",
    )
    parser.add_argument(
        "--max-concurrency",
        type=int,
        default=20,
        help="Maximum concurrent workers.",
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=0.0,
        help="Interval (seconds) between requests in each worker.",
    )
    args = parser.parse_args()
    cfg = RunConfig(
        total_time_sec=args.total_time_sec,
        max_requests=args.max_requests,
        max_concurrency=args.max_concurrency,
        interval_sec=args.interval,
    )
    validate_config(cfg)
    return cfg


def effective_config() -> RunConfig:
    cfg = CODE_CONFIG if USE_CODE_CONFIG else parse_args()
    validate_config(cfg)
    return cfg


def worker(
    parsed_url,
    interval_sec: float,
    deadline: Optional[float],
    enforce_budget: bool,
    budget: RequestBudget,
    result: WorkerResult,
) -> None:
    host = parsed_url.hostname or "localhost"
    port = parsed_url.port or (443 if parsed_url.scheme == "https" else 80)
    path = parsed_url.path or "/"
    if parsed_url.query:
        path += "?" + parsed_url.query

    conn_cls = http.client.HTTPSConnection if parsed_url.scheme == "https" else http.client.HTTPConnection
    conn = conn_cls(host, port=port, timeout=REQUEST_TIMEOUT_SEC)
    headers = {"Connection": "keep-alive"}

    while True:
        if deadline is not None and time.monotonic() >= deadline:
            break
        if enforce_budget and not budget.acquire():
            break

        result.sent += 1
        try:
            conn.request("GET", path, headers=headers)
            resp = conn.getresponse()
            payload = resp.read()
            result.bytes_read += len(payload)
            if 200 <= resp.status < 400:
                result.ok += 1
            else:
                result.failed += 1
        except Exception:
            result.failed += 1
            try:
                conn.close()
            finally:
                conn = conn_cls(host, port=port, timeout=REQUEST_TIMEOUT_SEC)

        if interval_sec > 0:
            time.sleep(interval_sec)

    try:
        conn.close()
    except Exception:
        pass


def main() -> None:
    cfg = effective_config()
    parsed = urlparse(TARGET_URL)
    if parsed.scheme not in {"http", "https"}:
        raise SystemExit("TARGET_URL must be http/https")

    time_limited = cfg.total_time_sec > 0
    deadline = time.monotonic() + cfg.total_time_sec if time_limited else None
    enforce_budget = not time_limited
    budget = RequestBudget(cfg.max_requests if enforce_budget else None)

    results = [WorkerResult() for _ in range(cfg.max_concurrency)]
    threads = []
    started_at = time.perf_counter()

    for i in range(cfg.max_concurrency):
        t = threading.Thread(
            target=worker,
            args=(parsed, cfg.interval_sec, deadline, enforce_budget, budget, results[i]),
            daemon=True,
        )
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    elapsed = time.perf_counter() - started_at
    total_sent = sum(r.sent for r in results)
    total_ok = sum(r.ok for r in results)
    total_failed = sum(r.failed for r in results)
    total_bytes = sum(r.bytes_read for r in results)
    rps = total_sent / elapsed if elapsed > 0 else 0.0

    print("=== Load Test Result ===")
    print(f"target            : {TARGET_URL}")
    print(f"total_time_sec    : {cfg.total_time_sec}")
    print(f"max_requests      : {cfg.max_requests}")
    print(f"max_concurrency   : {cfg.max_concurrency}")
    print(f"interval_sec      : {cfg.interval_sec}")
    print(f"priority          : total_time")
    print(f"elapsed(s)        : {elapsed:.2f}")
    print(f"sent              : {total_sent}")
    print(f"ok                : {total_ok}")
    print(f"failed            : {total_failed}")
    print(f"throughput        : {rps:.2f} req/s")
    print(f"bytes received    : {total_bytes}")


if __name__ == "__main__":
    main()
