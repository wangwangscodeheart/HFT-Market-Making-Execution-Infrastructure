window.P03_QUOTE_RUNNER = {
  "schema": "p03.quote-runner.v1",
  "scenario": "quote-cycle",
  "max_book_age": 10,
  "market_origin": "InterfaceFixture",
  "execution_origin": "LocalSimulator",
  "historical_trace": false,
  "time_unit": "logical_step",
  "money_unit": "local_model_unit",
  "peer_initial_budget": 1000000,
  "passed": true,
  "runner_stopped": false,
  "cleanup_exhausted": false,
  "admission_checks": [],
  "frames": [
    {
      "ordinal": 1,
      "at": 1,
      "input": {
        "type": "MarketPulse",
        "source": "fixture.book",
        "epoch": 1,
        "sequence": 1,
        "received_at": 1,
        "bid": 499,
        "ask": 501,
        "bid_quantity": 10,
        "ask_quantity": 10,
        "healthy": true,
        "subscription_generation": 1,
        "session_second": 32460
      },
      "error": "None",
      "completed": true,
      "mode": 0,
      "core_positions": [
        0,
        0,
        0,
        0
      ],
      "peer_positions": [
        0,
        0,
        0,
        0
      ],
      "peer_revision": 2,
      "pending_reports": 0,
      "pending_delivery": 0,
      "query_blocked": false,
      "delivery_loss": false,
      "core_required": 204,
      "peer_available": 999796,
      "cleanup_lane": false,
      "query_aligned": null,
      "decision": {
        "error": "None",
        "withdraw": false,
        "generation": 1
      },
      "orders": [
        {
          "id": "7:1",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 0,
            "offset": 0,
            "price": 499,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            1,
            0,
            0
          ],
          "execution": 1
        },
        {
          "id": "7:2",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 1,
            "offset": 0,
            "price": 501,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            1,
            0,
            0
          ],
          "execution": 1
        }
      ],
      "calls": [
        {
          "input": {
            "type": "ReferenceMark",
            "price": 500
          },
          "core_sequence": 1,
          "ingress": 1,
          "error": "None"
        },
        {
          "input": {
            "type": "SubmitOrder",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            }
          },
          "core_sequence": 2,
          "ingress": 2,
          "error": "None"
        },
        {
          "input": {
            "type": "WorkingReport",
            "order": "7:1"
          },
          "core_sequence": 3,
          "ingress": 3,
          "error": "None"
        },
        {
          "input": {
            "type": "SubmitOrder",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            }
          },
          "core_sequence": 4,
          "ingress": 4,
          "error": "None"
        },
        {
          "input": {
            "type": "WorkingReport",
            "order": "7:2"
          },
          "core_sequence": 5,
          "ingress": 5,
          "error": "None"
        }
      ],
      "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
    },
    {
      "ordinal": 2,
      "at": 2,
      "input": {
        "type": "MarketPulse",
        "source": "fixture.book",
        "epoch": 1,
        "sequence": 2,
        "received_at": 2,
        "bid": 498,
        "ask": 502,
        "bid_quantity": 10,
        "ask_quantity": 10,
        "healthy": true,
        "subscription_generation": 1,
        "session_second": 32460
      },
      "error": "None",
      "completed": true,
      "mode": 0,
      "core_positions": [
        0,
        0,
        0,
        0
      ],
      "peer_positions": [
        0,
        0,
        0,
        0
      ],
      "peer_revision": 6,
      "pending_reports": 0,
      "pending_delivery": 0,
      "query_blocked": false,
      "delivery_loss": false,
      "core_required": 204,
      "peer_available": 999796,
      "cleanup_lane": false,
      "query_aligned": null,
      "decision": {
        "error": "None",
        "withdraw": false,
        "generation": 2
      },
      "orders": [
        {
          "id": "7:1",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 0,
            "offset": 0,
            "price": 499,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            0,
            1,
            0
          ],
          "execution": 4
        },
        {
          "id": "7:2",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 1,
            "offset": 0,
            "price": 501,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            0,
            1,
            0
          ],
          "execution": 4
        },
        {
          "id": "7:3",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 0,
            "offset": 0,
            "price": 498,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            1,
            0,
            0
          ],
          "execution": 1
        },
        {
          "id": "7:4",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 1,
            "offset": 0,
            "price": 502,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            1,
            0,
            0
          ],
          "execution": 1
        }
      ],
      "calls": [
        {
          "input": {
            "type": "ReferenceMark",
            "price": 500
          },
          "core_sequence": 6,
          "ingress": 6,
          "error": "None"
        },
        {
          "input": {
            "type": "RequestCancel",
            "order": "7:1"
          },
          "core_sequence": 7,
          "ingress": 7,
          "error": "None"
        },
        {
          "input": {
            "type": "CancelReport",
            "order": "7:1",
            "cancel_state": 2
          },
          "core_sequence": 8,
          "ingress": 8,
          "error": "None"
        },
        {
          "input": {
            "type": "TerminalReport",
            "order": "7:1",
            "terminal_kind": 1,
            "exact_quantity": 0
          },
          "core_sequence": 9,
          "ingress": 9,
          "error": "None"
        },
        {
          "input": {
            "type": "RequestCancel",
            "order": "7:2"
          },
          "core_sequence": 10,
          "ingress": 10,
          "error": "None"
        },
        {
          "input": {
            "type": "CancelReport",
            "order": "7:2",
            "cancel_state": 2
          },
          "core_sequence": 11,
          "ingress": 11,
          "error": "None"
        },
        {
          "input": {
            "type": "TerminalReport",
            "order": "7:2",
            "terminal_kind": 1,
            "exact_quantity": 0
          },
          "core_sequence": 12,
          "ingress": 12,
          "error": "None"
        },
        {
          "input": {
            "type": "SubmitOrder",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 498,
              "quantity": 1
            }
          },
          "core_sequence": 13,
          "ingress": 13,
          "error": "None"
        },
        {
          "input": {
            "type": "WorkingReport",
            "order": "7:3"
          },
          "core_sequence": 14,
          "ingress": 14,
          "error": "None"
        },
        {
          "input": {
            "type": "SubmitOrder",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 502,
              "quantity": 1
            }
          },
          "core_sequence": 15,
          "ingress": 15,
          "error": "None"
        },
        {
          "input": {
            "type": "WorkingReport",
            "order": "7:4"
          },
          "core_sequence": 16,
          "ingress": 16,
          "error": "None"
        }
      ],
      "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|16|2|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:2|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:498:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:502:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:0:0:0|fund:3:0:0:0|fund:4:100:0:2|fund:5:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:2|admitted:2|ids:5:0:17:0:3:0:6:0"
    },
    {
      "ordinal": 3,
      "at": 3,
      "input": {
        "type": "LocalFillInput",
        "order": "7:3",
        "trade_id": "runner-export-fill",
        "quantity": 1,
        "price": 498
      },
      "error": "None",
      "completed": true,
      "mode": 0,
      "core_positions": [
        1,
        0,
        0,
        0
      ],
      "peer_positions": [
        1,
        0,
        0,
        0
      ],
      "peer_revision": 7,
      "pending_reports": 0,
      "pending_delivery": 0,
      "query_blocked": false,
      "delivery_loss": false,
      "core_required": 204,
      "peer_available": 999796,
      "cleanup_lane": false,
      "query_aligned": null,
      "decision": null,
      "orders": [
        {
          "id": "7:1",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 0,
            "offset": 0,
            "price": 499,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            0,
            1,
            0
          ],
          "execution": 4
        },
        {
          "id": "7:2",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 1,
            "offset": 0,
            "price": 501,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            0,
            1,
            0
          ],
          "execution": 4
        },
        {
          "id": "7:3",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 0,
            "offset": 0,
            "price": 498,
            "quantity": 1
          },
          "parts": [
            1,
            0,
            0,
            0,
            0
          ],
          "execution": 3
        },
        {
          "id": "7:4",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 1,
            "offset": 0,
            "price": 502,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            1,
            0,
            0
          ],
          "execution": 1
        }
      ],
      "calls": [
        {
          "input": {
            "type": "TradeReport",
            "order": "7:3",
            "quantity": 1,
            "price": 498,
            "trade_id": "runner-export-fill",
            "channel": 0,
            "source": "localsim.trades"
          },
          "core_sequence": 17,
          "ingress": 17,
          "error": "None"
        }
      ],
      "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|17|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:2|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:498:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:502:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"runner-export-fill\":3:1:498:0|fund:1:0:0:0|fund:2:0:0:0|fund:3:0:0:0|fund:4:0:100:2|fund:5:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:2|admitted:2|ids:5:0:18:0:3:0:6:0"
    },
    {
      "ordinal": 4,
      "at": 4,
      "input": {
        "type": "MarketPulse",
        "source": "fixture.book",
        "epoch": 1,
        "sequence": 3,
        "received_at": 4,
        "bid": 498,
        "ask": 502,
        "bid_quantity": 10,
        "ask_quantity": 10,
        "healthy": false,
        "subscription_generation": 1,
        "session_second": 32460
      },
      "error": "Blocked",
      "completed": true,
      "mode": 0,
      "core_positions": [
        1,
        0,
        0,
        0
      ],
      "peer_positions": [
        1,
        0,
        0,
        0
      ],
      "peer_revision": 8,
      "pending_reports": 0,
      "pending_delivery": 0,
      "query_blocked": false,
      "delivery_loss": false,
      "core_required": 102,
      "peer_available": 999898,
      "cleanup_lane": false,
      "query_aligned": null,
      "decision": {
        "error": "Blocked",
        "withdraw": true,
        "generation": 0
      },
      "orders": [
        {
          "id": "7:1",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 0,
            "offset": 0,
            "price": 499,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            0,
            1,
            0
          ],
          "execution": 4
        },
        {
          "id": "7:2",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 1,
            "offset": 0,
            "price": 501,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            0,
            1,
            0
          ],
          "execution": 4
        },
        {
          "id": "7:3",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 0,
            "offset": 0,
            "price": 498,
            "quantity": 1
          },
          "parts": [
            1,
            0,
            0,
            0,
            0
          ],
          "execution": 3
        },
        {
          "id": "7:4",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 1,
            "offset": 0,
            "price": 502,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            0,
            1,
            0
          ],
          "execution": 4
        }
      ],
      "calls": [
        {
          "input": {
            "type": "RequestCancel",
            "order": "7:4"
          },
          "core_sequence": 18,
          "ingress": 18,
          "error": "None"
        },
        {
          "input": {
            "type": "CancelReport",
            "order": "7:4",
            "cancel_state": 2
          },
          "core_sequence": 19,
          "ingress": 19,
          "error": "None"
        },
        {
          "input": {
            "type": "TerminalReport",
            "order": "7:4",
            "terminal_kind": 1,
            "exact_quantity": 0
          },
          "core_sequence": 20,
          "ingress": 20,
          "error": "None"
        }
      ],
      "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|20|4|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:2|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:498:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:502:1:0:0:0:1:0:0:1:1:4:2;action:3:2;current:3;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"runner-export-fill\":3:1:498:0|fund:1:0:0:0|fund:2:0:0:0|fund:3:0:0:0|fund:4:0:100:2|fund:5:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:2|admitted:2|ids:5:0:21:0:4:0:6:0"
    },
    {
      "ordinal": 5,
      "at": 5,
      "input": {
        "type": "StopLocalQuotes"
      },
      "error": "None",
      "completed": true,
      "mode": 1,
      "core_positions": [
        1,
        0,
        0,
        0
      ],
      "peer_positions": [
        1,
        0,
        0,
        0
      ],
      "peer_revision": 8,
      "pending_reports": 0,
      "pending_delivery": 0,
      "query_blocked": false,
      "delivery_loss": false,
      "core_required": 102,
      "peer_available": 999898,
      "cleanup_lane": false,
      "query_aligned": null,
      "decision": null,
      "orders": [
        {
          "id": "7:1",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 0,
            "offset": 0,
            "price": 499,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            0,
            1,
            0
          ],
          "execution": 4
        },
        {
          "id": "7:2",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 1,
            "offset": 0,
            "price": 501,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            0,
            1,
            0
          ],
          "execution": 4
        },
        {
          "id": "7:3",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 0,
            "offset": 0,
            "price": 498,
            "quantity": 1
          },
          "parts": [
            1,
            0,
            0,
            0,
            0
          ],
          "execution": 3
        },
        {
          "id": "7:4",
          "intent": {
            "account": "fixture.account",
            "instrument": "rb_FIXTURE",
            "day": 20261105,
            "side": 1,
            "offset": 0,
            "price": 502,
            "quantity": 1
          },
          "parts": [
            0,
            0,
            0,
            1,
            0
          ],
          "execution": 4
        }
      ],
      "calls": [
        {
          "input": {
            "type": "KillRequested"
          },
          "core_sequence": 21,
          "ingress": 21,
          "error": "None"
        }
      ],
      "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|21|5|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:2|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:498:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:502:1:0:0:0:1:0:0:1:1:4:2;action:3:2;current:3;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"runner-export-fill\":3:1:498:0|fund:1:0:0:0|fund:2:0:0:0|fund:3:0:0:0|fund:4:0:100:2|fund:5:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:2|admitted:2|ids:5:0:22:0:4:0:6:0"
    }
  ],
  "provenance": {
    "run": "2026-11-08-market-state-debug",
    "configuration": "Debug",
    "sha256": "3a3357f3d70080ecfe0d15a5e154e7865e568f590edb970557e8397cc5004b38",
    "producer": "p03_export_quote_runner"
  },
  "expiry": {
    "schema": "p03.quote-runner.v1",
    "scenario": "market-expiry",
    "max_book_age": 10,
    "market_origin": "InterfaceFixture",
    "execution_origin": "LocalSimulator",
    "historical_trace": false,
    "time_unit": "logical_step",
    "money_unit": "local_model_unit",
    "peer_initial_budget": 1000000,
    "passed": true,
    "runner_stopped": false,
    "cleanup_exhausted": false,
    "admission_checks": [],
    "frames": [
      {
        "ordinal": 1,
        "at": 1,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 1,
          "received_at": 1,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 1
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 1,
            "ingress": 1,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 0,
                "offset": 0,
                "price": 499,
                "quantity": 1
              }
            },
            "core_sequence": 2,
            "ingress": 2,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:1"
            },
            "core_sequence": 3,
            "ingress": 3,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 1,
                "offset": 0,
                "price": 501,
                "quantity": 1
              }
            },
            "core_sequence": 4,
            "ingress": 4,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:2"
            },
            "core_sequence": 5,
            "ingress": 5,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 2,
        "at": 11,
        "input": {
          "type": "LocalQuoteTimer"
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 3,
        "at": 12,
        "input": {
          "type": "LocalQuoteTimer"
        },
        "error": "StaleMark",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 0,
        "peer_available": 1000000,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:1"
            },
            "core_sequence": 6,
            "ingress": 6,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:1",
              "cancel_state": 2
            },
            "core_sequence": 7,
            "ingress": 7,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:1",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 8,
            "ingress": 8,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:2"
            },
            "core_sequence": 9,
            "ingress": 9,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:2",
              "cancel_state": 2
            },
            "core_sequence": 10,
            "ingress": 10,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:2",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 11,
            "ingress": 11,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|11|12|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:0:0:0|fund:3:0:0:0|budget:0:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:12:0:3:0:4:0"
      },
      {
        "ordinal": 4,
        "at": 13,
        "input": {
          "type": "LocalQuoteTimer"
        },
        "error": "StaleMark",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 0,
        "peer_available": 1000000,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|11|12|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:0:0:0|fund:3:0:0:0|budget:0:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:12:0:3:0:4:0"
      },
      {
        "ordinal": 5,
        "at": 14,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 2,
          "received_at": 14,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 6,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 2
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:3",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:4",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 12,
            "ingress": 12,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 0,
                "offset": 0,
                "price": 499,
                "quantity": 1
              }
            },
            "core_sequence": 13,
            "ingress": 13,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:3"
            },
            "core_sequence": 14,
            "ingress": 14,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 1,
                "offset": 0,
                "price": 501,
                "quantity": 1
              }
            },
            "core_sequence": 15,
            "ingress": 15,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:4"
            },
            "core_sequence": 16,
            "ingress": 16,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|16|14|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:14|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:0:0:0|fund:3:0:0:0|fund:4:100:0:2|fund:5:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:14|admitted:14|ids:5:0:17:0:3:0:6:0"
      },
      {
        "ordinal": 6,
        "at": 15,
        "input": {
          "type": "StopLocalQuotes"
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 8,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 0,
        "peer_available": 1000000,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:3",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:4",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "KillRequested"
            },
            "core_sequence": 17,
            "ingress": 17,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:3"
            },
            "core_sequence": 18,
            "ingress": 18,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:3",
              "cancel_state": 2
            },
            "core_sequence": 19,
            "ingress": 19,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:3",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 20,
            "ingress": 20,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:4"
            },
            "core_sequence": 21,
            "ingress": 21,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:4",
              "cancel_state": 2
            },
            "core_sequence": 22,
            "ingress": 22,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:4",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 23,
            "ingress": 23,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|23|15|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:14|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:3:2;current:3;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:4:2;current:4;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:0:0:0|fund:3:0:0:0|fund:4:0:0:0|fund:5:0:0:0|budget:0:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:14|admitted:14|ids:5:0:24:0:5:0:6:0"
      }
    ],
    "provenance": {
      "run": "2026-11-08-market-state-debug",
      "configuration": "Debug",
      "sha256": "4f2c48f98eec8178c6aaebaa9f92b9033ce8912b7755b371fdc4afd04c854325",
      "producer": "p03_export_quote_runner"
    }
  },
  "query_gap": {
    "schema": "p03.quote-runner.v1",
    "scenario": "query-gap",
    "max_book_age": 10,
    "market_origin": "InterfaceFixture",
    "execution_origin": "LocalSimulator",
    "historical_trace": false,
    "time_unit": "logical_step",
    "money_unit": "local_model_unit",
    "peer_initial_budget": 1000000,
    "passed": true,
    "runner_stopped": false,
    "cleanup_exhausted": false,
    "admission_checks": [],
    "frames": [
      {
        "ordinal": 1,
        "at": 1,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 1,
          "received_at": 1,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 1
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 1,
            "ingress": 1,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 0,
                "offset": 0,
                "price": 499,
                "quantity": 1
              }
            },
            "core_sequence": 2,
            "ingress": 2,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:1"
            },
            "core_sequence": 3,
            "ingress": 3,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 1,
                "offset": 0,
                "price": 501,
                "quantity": 1
              }
            },
            "core_sequence": 4,
            "ingress": 4,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:2"
            },
            "core_sequence": 5,
            "ingress": 5,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 2,
        "at": 2,
        "input": {
          "type": "LocalDeferredFillInput",
          "order": "7:1",
          "trade_id": "export-query-gap",
          "quantity": 1,
          "price": 499
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 3,
        "pending_reports": 1,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 3,
        "at": 3,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 2,
          "received_at": 3,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "Blocked",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 5,
        "pending_reports": 1,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "Blocked",
          "withdraw": true,
          "generation": 0
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:1"
            },
            "core_sequence": 6,
            "ingress": 6,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:1",
              "cancel_state": 3
            },
            "core_sequence": 7,
            "ingress": 7,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:2"
            },
            "core_sequence": 8,
            "ingress": 8,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:2",
              "cancel_state": 2
            },
            "core_sequence": 9,
            "ingress": 9,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:2",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 10,
            "ingress": 10,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|10|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:3;action:1:3;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:11:0:3:0:4:0"
      },
      {
        "ordinal": 4,
        "at": 4,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 100,
          "omit_last_trade": true,
          "bypass_backpressure_for_test": false
        },
        "error": "Blocked",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 5,
        "pending_reports": 1,
        "pending_delivery": 0,
        "query_blocked": true,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|10|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:3;action:1:3;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:11:0:3:0:4:0"
      },
      {
        "ordinal": 5,
        "at": 5,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 101,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": false
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 5,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": false,
        "query_aligned": true,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "export-query-gap",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 11,
            "ingress": 11,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|11|5|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:3;action:1:3;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"export-query-gap\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:12:0:3:0:4:0"
      },
      {
        "ordinal": 6,
        "at": 6,
        "input": {
          "type": "LocalHeldReportInput",
          "index": 0
        },
        "error": "Duplicate",
        "completed": true,
        "mode": 0,
        "core_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 5,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "export-query-gap",
              "channel": 0,
              "source": "localsim.trades"
            },
            "core_sequence": 12,
            "ingress": 12,
            "error": "Duplicate"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|12|6|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:3;action:1:3;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"export-query-gap\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:13:0:3:0:4:0"
      },
      {
        "ordinal": 7,
        "at": 7,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 3,
          "received_at": 7,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 7,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 206,
        "peer_available": 999794,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 2
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:3",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:4",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 13,
            "ingress": 13,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 0,
                "offset": 0,
                "price": 499,
                "quantity": 1
              }
            },
            "core_sequence": 14,
            "ingress": 14,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:3"
            },
            "core_sequence": 15,
            "ingress": 15,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 1,
                "offset": 1,
                "price": 501,
                "quantity": 1
              }
            },
            "core_sequence": 16,
            "ingress": 16,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:4"
            },
            "core_sequence": 17,
            "ingress": 17,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|17|7|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|1|0|0|0|mark:500:7|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:3;action:1:3;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"export-query-gap\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|fund:4:100:0:2|fund:5:0:0:2|budget:206:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:7|admitted:7|ids:5:0:18:0:3:0:6:0"
      },
      {
        "ordinal": 8,
        "at": 8,
        "input": {
          "type": "StopLocalQuotes"
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 9,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:3",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:4",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "KillRequested"
            },
            "core_sequence": 18,
            "ingress": 18,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:3"
            },
            "core_sequence": 19,
            "ingress": 19,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:3",
              "cancel_state": 2
            },
            "core_sequence": 20,
            "ingress": 20,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:3",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 21,
            "ingress": 21,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:4"
            },
            "core_sequence": 22,
            "ingress": 22,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:4",
              "cancel_state": 2
            },
            "core_sequence": 23,
            "ingress": 23,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:4",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 24,
            "ingress": 24,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|24|8|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:7|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:3;action:1:3;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:3:2;current:3;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:0:0:0:1:0:0:1:1:4:2;action:4:2;current:4;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"export-query-gap\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|fund:4:0:0:0|fund:5:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:7|admitted:7|ids:5:0:25:0:5:0:6:0"
      }
    ],
    "provenance": {
      "run": "2026-11-08-market-state-debug",
      "configuration": "Debug",
      "sha256": "e896f070ce130585a3aa5af54087ab7a75daeaacd0999b21dd8be83125256eac",
      "producer": "p03_export_quote_runner"
    }
  },
  "retry": {
    "schema": "p03.quote-runner.v1",
    "scenario": "evidence-retry",
    "max_book_age": 10,
    "market_origin": "InterfaceFixture",
    "execution_origin": "LocalSimulator",
    "historical_trace": false,
    "time_unit": "logical_step",
    "money_unit": "local_model_unit",
    "peer_initial_budget": 1000000,
    "passed": true,
    "runner_stopped": false,
    "cleanup_exhausted": false,
    "admission_checks": [],
    "frames": [
      {
        "ordinal": 1,
        "at": 1,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 1,
          "received_at": 1,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 1
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 1,
            "ingress": 1,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 0,
                "offset": 0,
                "price": 499,
                "quantity": 1
              }
            },
            "core_sequence": 2,
            "ingress": 2,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:1"
            },
            "core_sequence": 3,
            "ingress": 3,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 1,
                "offset": 0,
                "price": 501,
                "quantity": 1
              }
            },
            "core_sequence": 4,
            "ingress": 4,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:2"
            },
            "core_sequence": 5,
            "ingress": 5,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 2,
        "at": 2,
        "input": {
          "type": "LocalDeferredFillInput",
          "order": "7:1",
          "trade_id": "retry-buy",
          "quantity": 1,
          "price": 499
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 3,
        "pending_reports": 1,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 3,
        "at": 3,
        "input": {
          "type": "LocalDeferredFillInput",
          "order": "7:2",
          "trade_id": "retry-sell",
          "quantity": 1,
          "price": 501
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 2,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 4,
        "at": 4,
        "input": {
          "type": "LocalReportFailureInput",
          "accepted_before_failure": 0
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 2,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 5,
        "at": 5,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 100,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": false
        },
        "error": "InjectedFailure",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 2,
        "pending_delivery": 2,
        "query_blocked": true,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "retry-buy",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 6,
            "ingress": 6,
            "error": "InjectedFailure"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|6|5|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:7:0:1:0:4:0"
      },
      {
        "ordinal": 6,
        "at": 6,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 2,
          "received_at": 6,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "Blocked",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 2,
        "pending_delivery": 2,
        "query_blocked": true,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "Blocked",
          "withdraw": true,
          "generation": 0
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|6|5|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:7:0:1:0:4:0"
      },
      {
        "ordinal": 7,
        "at": 7,
        "input": {
          "type": "LocalRetryEvidenceInput"
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": true,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "retry-buy",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 7,
            "ingress": 7,
            "error": "None"
          },
          {
            "input": {
              "type": "TradeReport",
              "order": "7:2",
              "quantity": 1,
              "price": 501,
              "trade_id": "retry-sell",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 8,
            "ingress": 8,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|8|7|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|1|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"retry-buy\":1:1:499:0|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"retry-sell\":2:1:501:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:100:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:9:0:1:0:4:0"
      },
      {
        "ordinal": 8,
        "at": 8,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 3,
          "received_at": 8,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "Blocked",
        "completed": true,
        "mode": 0,
        "core_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": true,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "Blocked",
          "withdraw": true,
          "generation": 0
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|8|7|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|1|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"retry-buy\":1:1:499:0|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"retry-sell\":2:1:501:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:100:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:9:0:1:0:4:0"
      },
      {
        "ordinal": 9,
        "at": 9,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 101,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": false
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": true,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "retry-buy",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 9,
            "ingress": 9,
            "error": "Duplicate"
          },
          {
            "input": {
              "type": "TradeReport",
              "order": "7:2",
              "quantity": 1,
              "price": 501,
              "trade_id": "retry-sell",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 10,
            "ingress": 10,
            "error": "Duplicate"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|10|9|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|1|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"retry-buy\":1:1:499:0|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"retry-sell\":2:1:501:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:100:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:11:0:1:0:4:0"
      },
      {
        "ordinal": 10,
        "at": 10,
        "input": {
          "type": "StopLocalQuotes"
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_positions": [
          1,
          0,
          1,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          }
        ],
        "calls": [
          {
            "input": {
              "type": "KillRequested"
            },
            "core_sequence": 11,
            "ingress": 11,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|11|10|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|1|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"retry-buy\":1:1:499:0|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"retry-sell\":2:1:501:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:100:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:12:0:1:0:4:0"
      }
    ],
    "provenance": {
      "run": "2026-11-08-market-state-debug",
      "configuration": "Debug",
      "sha256": "f53d2688c55e80ea2147066b4c78fd7b03c6e4810e61fde873ad022ca5626a81",
      "producer": "p03_export_quote_runner"
    }
  },
  "overflow": {
    "schema": "p03.quote-runner.v1",
    "scenario": "evidence-overflow",
    "max_book_age": 10,
    "market_origin": "InterfaceFixture",
    "execution_origin": "LocalSimulator",
    "historical_trace": false,
    "time_unit": "logical_step",
    "money_unit": "local_model_unit",
    "peer_initial_budget": 1000000,
    "passed": true,
    "runner_stopped": false,
    "cleanup_exhausted": false,
    "admission_checks": [],
    "frames": [
      {
        "ordinal": 1,
        "at": 1,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 1,
          "received_at": 1,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_revision": 1,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 1002,
        "peer_available": 998998,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 1
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 1,
            "ingress": 1,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 1,
                "offset": 1,
                "price": 501,
                "quantity": 1
              }
            },
            "core_sequence": 2,
            "ingress": 2,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:1"
            },
            "core_sequence": 3,
            "ingress": 3,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|3|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|10|0|0|0|1|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:1000:0|fund:2:0:0:2|budget:1002:0:0:0|quarantine:0:0|admitted:1|ids:2:0:4:0:1:0:3:0"
      },
      {
        "ordinal": 2,
        "at": 2,
        "input": {
          "type": "LocalReportFailureInput",
          "accepted_before_failure": 0
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_revision": 1,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 1002,
        "peer_available": 998998,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|3|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|10|0|0|0|1|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:1000:0|fund:2:0:0:2|budget:1002:0:0:0|quarantine:0:0|admitted:1|ids:2:0:4:0:1:0:3:0"
      },
      {
        "ordinal": 3,
        "at": 3,
        "input": {
          "type": "LocalFillInput",
          "order": "7:1",
          "trade_id": "overflow-export-close",
          "quantity": 1,
          "price": 501
        },
        "error": "InjectedFailure",
        "completed": true,
        "mode": 0,
        "core_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 1,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 1002,
        "peer_available": 998998,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 501,
              "trade_id": "overflow-export-close",
              "channel": 0,
              "source": "localsim.trades"
            },
            "core_sequence": 4,
            "ingress": 4,
            "error": "InjectedFailure"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|4|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|10|0|0|0|1|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:1000:0|fund:2:0:0:2|budget:1002:0:0:0|quarantine:0:0|admitted:1|ids:2:0:5:0:1:0:3:0"
      },
      {
        "ordinal": 4,
        "at": 4,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 1,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": true
        },
        "error": "Blocked",
        "completed": true,
        "mode": 0,
        "core_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 2,
        "query_blocked": true,
        "delivery_loss": false,
        "core_required": 1002,
        "peer_available": 998998,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|4|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|10|0|0|0|1|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:1000:0|fund:2:0:0:2|budget:1002:0:0:0|quarantine:0:0|admitted:1|ids:2:0:5:0:1:0:3:0"
      },
      {
        "ordinal": 5,
        "at": 5,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 2,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": true
        },
        "error": "Blocked",
        "completed": true,
        "mode": 0,
        "core_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 3,
        "query_blocked": true,
        "delivery_loss": false,
        "core_required": 1002,
        "peer_available": 998998,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|4|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|10|0|0|0|1|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:1000:0|fund:2:0:0:2|budget:1002:0:0:0|quarantine:0:0|admitted:1|ids:2:0:5:0:1:0:3:0"
      },
      {
        "ordinal": 6,
        "at": 6,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 3,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": true
        },
        "error": "Capacity",
        "completed": true,
        "mode": 0,
        "core_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 3,
        "query_blocked": true,
        "delivery_loss": true,
        "core_required": 1002,
        "peer_available": 998998,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|4|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|10|0|0|0|1|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:1000:0|fund:2:0:0:2|budget:1002:0:0:0|quarantine:0:0|admitted:1|ids:2:0:5:0:1:0:3:0"
      },
      {
        "ordinal": 7,
        "at": 7,
        "input": {
          "type": "LocalRetryEvidenceInput"
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": true,
        "delivery_loss": true,
        "core_required": 1002,
        "peer_available": 998998,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 501,
              "trade_id": "overflow-export-close",
              "channel": 0,
              "source": "localsim.trades"
            },
            "core_sequence": 5,
            "ingress": 5,
            "error": "None"
          },
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 501,
              "trade_id": "overflow-export-close",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 6,
            "ingress": 6,
            "error": "Duplicate"
          },
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 501,
              "trade_id": "overflow-export-close",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 7,
            "ingress": 7,
            "error": "Duplicate"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|7|7|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|9|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"overflow-export-close\":1:1:501:0|fund:1:0:1000:0|fund:2:0:0:2|budget:1002:0:0:0|quarantine:0:0|admitted:1|ids:2:0:8:0:1:0:3:0"
      },
      {
        "ordinal": 8,
        "at": 8,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 4,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": false
        },
        "error": "Blocked",
        "completed": true,
        "mode": 0,
        "core_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": true,
        "delivery_loss": true,
        "core_required": 1002,
        "peer_available": 998998,
        "cleanup_lane": false,
        "query_aligned": true,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 501,
              "trade_id": "overflow-export-close",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 8,
            "ingress": 8,
            "error": "Duplicate"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|8|8|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|9|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"overflow-export-close\":1:1:501:0|fund:1:0:1000:0|fund:2:0:0:2|budget:1002:0:0:0|quarantine:0:0|admitted:1|ids:2:0:9:0:1:0:3:0"
      },
      {
        "ordinal": 9,
        "at": 9,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 2,
          "received_at": 9,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "Blocked",
        "completed": true,
        "mode": 0,
        "core_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": true,
        "delivery_loss": true,
        "core_required": 1002,
        "peer_available": 998998,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "Blocked",
          "withdraw": true,
          "generation": 0
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|8|8|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|9|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"overflow-export-close\":1:1:501:0|fund:1:0:1000:0|fund:2:0:0:2|budget:1002:0:0:0|quarantine:0:0|admitted:1|ids:2:0:9:0:1:0:3:0"
      },
      {
        "ordinal": 10,
        "at": 10,
        "input": {
          "type": "StopLocalQuotes"
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": true,
        "delivery_loss": true,
        "core_required": 1002,
        "peer_available": 998998,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          }
        ],
        "calls": [
          {
            "input": {
              "type": "KillRequested"
            },
            "core_sequence": 9,
            "ingress": 9,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|9|10|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|9|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"overflow-export-close\":1:1:501:0|fund:1:0:1000:0|fund:2:0:0:2|budget:1002:0:0:0|quarantine:0:0|admitted:1|ids:2:0:10:0:1:0:3:0"
      }
    ],
    "provenance": {
      "run": "2026-11-08-market-state-debug",
      "configuration": "Debug",
      "sha256": "e58e0c91d4183b682a58aa0801a26b209478d157ce127154ca4a583cfc3fcce3",
      "producer": "p03_export_quote_runner"
    }
  },
  "cleanup": {
    "schema": "p03.quote-runner.v1",
    "scenario": "capacity-cleanup",
    "max_book_age": 10,
    "market_origin": "InterfaceFixture",
    "execution_origin": "LocalSimulator",
    "historical_trace": false,
    "time_unit": "logical_step",
    "money_unit": "local_model_unit",
    "peer_initial_budget": 1000000,
    "passed": true,
    "runner_stopped": true,
    "cleanup_exhausted": true,
    "admission_checks": [
      {
        "at": 4,
        "expected": "Capacity",
        "actual": "Capacity",
        "frames_before": 3,
        "frames_after": 3,
        "core_unchanged": true
      },
      {
        "at": 8,
        "expected": "Halted",
        "actual": "Halted",
        "frames_before": 6,
        "frames_after": 6,
        "core_unchanged": true
      },
      {
        "at": 9,
        "expected": "Capacity",
        "actual": "Capacity",
        "frames_before": 6,
        "frames_after": 6,
        "core_unchanged": true
      }
    ],
    "experiment_replay_verified": true,
    "input_attempts": [
      {
        "ordinal": 1,
        "at": 1,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 1,
          "received_at": 1,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "frames_before": 0,
        "frames_after": 1,
        "stopped": false,
        "cleanup_exhausted": false,
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 2,
        "at": 2,
        "input": {
          "type": "LocalReportFailureInput",
          "accepted_before_failure": 0
        },
        "error": "None",
        "frames_before": 1,
        "frames_after": 2,
        "stopped": false,
        "cleanup_exhausted": false,
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 3,
        "at": 3,
        "input": {
          "type": "LocalFillInput",
          "order": "7:1",
          "trade_id": "cleanup-export-fill",
          "quantity": 1,
          "price": 499
        },
        "error": "InjectedFailure",
        "frames_before": 2,
        "frames_after": 3,
        "stopped": false,
        "cleanup_exhausted": false,
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|6|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:7:0:1:0:4:0"
      },
      {
        "ordinal": 4,
        "at": 4,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 2,
          "received_at": 4,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "Capacity",
        "frames_before": 3,
        "frames_after": 3,
        "stopped": true,
        "cleanup_exhausted": false,
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|6|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:7:0:1:0:4:0"
      },
      {
        "ordinal": 5,
        "at": 5,
        "input": {
          "type": "LocalRetryEvidenceInput"
        },
        "error": "None",
        "frames_before": 3,
        "frames_after": 4,
        "stopped": true,
        "cleanup_exhausted": false,
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|7|5|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"cleanup-export-fill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:8:0:1:0:4:0"
      },
      {
        "ordinal": 6,
        "at": 6,
        "input": {
          "type": "StopLocalQuotes"
        },
        "error": "None",
        "frames_before": 4,
        "frames_after": 5,
        "stopped": true,
        "cleanup_exhausted": false,
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|11|6|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"cleanup-export-fill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:12:0:2:0:4:0"
      },
      {
        "ordinal": 7,
        "at": 7,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 100,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": false
        },
        "error": "None",
        "frames_before": 5,
        "frames_after": 6,
        "stopped": true,
        "cleanup_exhausted": false,
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|12|7|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"cleanup-export-fill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:13:0:2:0:4:0"
      },
      {
        "ordinal": 8,
        "at": 8,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 3,
          "received_at": 8,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "Halted",
        "frames_before": 6,
        "frames_after": 6,
        "stopped": true,
        "cleanup_exhausted": false,
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|12|7|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"cleanup-export-fill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:13:0:2:0:4:0"
      },
      {
        "ordinal": 9,
        "at": 9,
        "input": {
          "type": "LocalRetryEvidenceInput"
        },
        "error": "Capacity",
        "frames_before": 6,
        "frames_after": 6,
        "stopped": true,
        "cleanup_exhausted": true,
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|12|7|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"cleanup-export-fill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:13:0:2:0:4:0"
      }
    ],
    "frames": [
      {
        "ordinal": 1,
        "at": 1,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 1,
          "received_at": 1,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 1
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 1,
            "ingress": 1,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 0,
                "offset": 0,
                "price": 499,
                "quantity": 1
              }
            },
            "core_sequence": 2,
            "ingress": 2,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:1"
            },
            "core_sequence": 3,
            "ingress": 3,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 1,
                "offset": 0,
                "price": 501,
                "quantity": 1
              }
            },
            "core_sequence": 4,
            "ingress": 4,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:2"
            },
            "core_sequence": 5,
            "ingress": 5,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 2,
        "at": 2,
        "input": {
          "type": "LocalReportFailureInput",
          "accepted_before_failure": 0
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 3,
        "at": 3,
        "input": {
          "type": "LocalFillInput",
          "order": "7:1",
          "trade_id": "cleanup-export-fill",
          "quantity": 1,
          "price": 499
        },
        "error": "InjectedFailure",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 3,
        "pending_reports": 0,
        "pending_delivery": 1,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "cleanup-export-fill",
              "channel": 0,
              "source": "localsim.trades"
            },
            "core_sequence": 6,
            "ingress": 6,
            "error": "InjectedFailure"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|6|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:7:0:1:0:4:0"
      },
      {
        "ordinal": 4,
        "at": 5,
        "input": {
          "type": "LocalRetryEvidenceInput"
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 3,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 204,
        "peer_available": 999796,
        "cleanup_lane": true,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "cleanup-export-fill",
              "channel": 0,
              "source": "localsim.trades"
            },
            "core_sequence": 7,
            "ingress": 7,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|7|5|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"cleanup-export-fill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:100:0:2|budget:204:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:8:0:1:0:4:0"
      },
      {
        "ordinal": 5,
        "at": 6,
        "input": {
          "type": "StopLocalQuotes"
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": true,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "KillRequested"
            },
            "core_sequence": 8,
            "ingress": 8,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:2"
            },
            "core_sequence": 9,
            "ingress": 9,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:2",
              "cancel_state": 2
            },
            "core_sequence": 10,
            "ingress": 10,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:2",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 11,
            "ingress": 11,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|11|6|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"cleanup-export-fill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:12:0:2:0:4:0"
      },
      {
        "ordinal": 6,
        "at": 7,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 100,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": false
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": true,
        "query_aligned": true,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "cleanup-export-fill",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 12,
            "ingress": 12,
            "error": "Duplicate"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|12|7|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:1:0:0:0:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"cleanup-export-fill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:13:0:2:0:4:0"
      }
    ],
    "provenance": {
      "run": "2026-11-08-market-state-debug",
      "configuration": "Debug",
      "sha256": "a0db10d56d5e6087b36113930899f41831e8c47d4932d6710bce153b714625ad",
      "producer": "p03_export_quote_runner"
    }
  },
  "late_fill": {
    "schema": "p03.quote-runner.v1",
    "scenario": "late-after-kill",
    "max_book_age": 10,
    "market_origin": "InterfaceFixture",
    "execution_origin": "LocalSimulator",
    "historical_trace": false,
    "time_unit": "logical_step",
    "money_unit": "local_model_unit",
    "peer_initial_budget": 1000000,
    "passed": true,
    "runner_stopped": true,
    "cleanup_exhausted": false,
    "admission_checks": [],
    "frames": [
      {
        "ordinal": 1,
        "at": 1,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 1,
          "received_at": 1,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 612,
        "peer_available": 999388,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 1
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              3,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              3,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 1,
            "ingress": 1,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 0,
                "offset": 0,
                "price": 499,
                "quantity": 3
              }
            },
            "core_sequence": 2,
            "ingress": 2,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:1"
            },
            "core_sequence": 3,
            "ingress": 3,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 1,
                "offset": 0,
                "price": 501,
                "quantity": 3
              }
            },
            "core_sequence": 4,
            "ingress": 4,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:2"
            },
            "core_sequence": 5,
            "ingress": 5,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:3:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:3:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:300:0:6|fund:3:300:0:6|budget:612:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 2,
        "at": 2,
        "input": {
          "type": "LocalDeferredFillInput",
          "order": "7:1",
          "trade_id": "late-after-kill",
          "quantity": 1,
          "price": 499
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 3,
        "pending_reports": 1,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 612,
        "peer_available": 999388,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              3,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              3,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:3:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:3:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:300:0:6|fund:3:300:0:6|budget:612:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 3,
        "at": 3,
        "input": {
          "type": "StopLocalQuotes"
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 5,
        "pending_reports": 1,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": true,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 3
            },
            "parts": [
              0,
              1,
              0,
              2,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              0,
              3,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "KillRequested"
            },
            "core_sequence": 6,
            "ingress": 6,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:1"
            },
            "core_sequence": 7,
            "ingress": 7,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:1",
              "cancel_state": 2
            },
            "core_sequence": 8,
            "ingress": 8,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:1",
              "terminal_kind": 1,
              "exact_quantity": 1
            },
            "core_sequence": 9,
            "ingress": 9,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:2"
            },
            "core_sequence": 10,
            "ingress": 10,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:2",
              "cancel_state": 2
            },
            "core_sequence": 11,
            "ingress": 11,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:2",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 12,
            "ingress": 12,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|12|3|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:3:0:1:1:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:3:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:13:0:3:0:4:0"
      },
      {
        "ordinal": 4,
        "at": 4,
        "input": {
          "type": "StopLocalQuotes"
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 5,
        "pending_reports": 1,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": true,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 3
            },
            "parts": [
              0,
              1,
              0,
              2,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              0,
              3,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "KillRequested"
            },
            "core_sequence": 13,
            "ingress": 13,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|13|4|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|0|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:3:0:1:1:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:3:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|fund:1:0:0:0|fund:2:100:0:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:14:0:3:0:4:0"
      },
      {
        "ordinal": 5,
        "at": 5,
        "input": {
          "type": "LocalHeldReportInput",
          "index": 0
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 5,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": true,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 3
            },
            "parts": [
              1,
              0,
              0,
              2,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              0,
              3,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "late-after-kill",
              "channel": 0,
              "source": "localsim.trades"
            },
            "core_sequence": 14,
            "ingress": 14,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|14|5|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:3:1:1:1:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:3:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"late-after-kill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:15:0:3:0:4:0"
      },
      {
        "ordinal": 6,
        "at": 6,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 100,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": false
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 5,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": true,
        "query_aligned": true,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 3
            },
            "parts": [
              1,
              0,
              0,
              2,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              0,
              3,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "late-after-kill",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 15,
            "ingress": 15,
            "error": "Duplicate"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|15|6|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:3:1:1:1:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:3:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"late-after-kill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:16:0:3:0:4:0"
      },
      {
        "ordinal": 7,
        "at": 7,
        "input": {
          "type": "LocalHeldReportInput",
          "index": 0
        },
        "error": "Duplicate",
        "completed": true,
        "mode": 1,
        "core_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_positions": [
          1,
          0,
          0,
          0
        ],
        "peer_revision": 5,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 102,
        "peer_available": 999898,
        "cleanup_lane": true,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 3
            },
            "parts": [
              1,
              0,
              0,
              2,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 0,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              0,
              3,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "late-after-kill",
              "channel": 0,
              "source": "localsim.trades"
            },
            "core_sequence": 16,
            "ingress": 16,
            "error": "Duplicate"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|16|7|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|1|0|0|0|0|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:3:1:1:1:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:0:501:3:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"late-after-kill\":1:1:499:0|fund:1:0:0:0|fund:2:0:100:2|fund:3:0:0:0|budget:102:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:17:0:3:0:4:0"
      }
    ],
    "provenance": {
      "run": "2026-11-08-market-state-debug",
      "configuration": "Debug",
      "sha256": "78c5c9a04bd2eaa505085b30d1b732f266d0291f9831288bb952c9d2a08ed825",
      "producer": "p03_export_quote_runner"
    }
  },
  "inventory": {
    "schema": "p03.quote-runner.v1",
    "scenario": "inventory-boundary",
    "max_book_age": 10,
    "market_origin": "InterfaceFixture",
    "execution_origin": "LocalSimulator",
    "historical_trace": false,
    "time_unit": "logical_step",
    "money_unit": "local_model_unit",
    "peer_initial_budget": 1000000,
    "passed": true,
    "runner_stopped": false,
    "cleanup_exhausted": false,
    "admission_checks": [],
    "frames": [
      {
        "ordinal": 1,
        "at": 1,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 1,
          "received_at": 1,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 2,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 1008,
        "peer_available": 998992,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 1
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              3,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 1,
            "ingress": 1,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 0,
                "offset": 0,
                "price": 499,
                "quantity": 1
              }
            },
            "core_sequence": 2,
            "ingress": 2,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:1"
            },
            "core_sequence": 3,
            "ingress": 3,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 1,
                "offset": 1,
                "price": 501,
                "quantity": 3
              }
            },
            "core_sequence": 4,
            "ingress": 4,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:2"
            },
            "core_sequence": 5,
            "ingress": 5,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|5|1|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|9|0|0|0|3|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:3:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|fund:1:0:900:0|fund:2:100:0:2|fund:3:0:0:6|budget:1008:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:6:0:1:0:4:0"
      },
      {
        "ordinal": 2,
        "at": 2,
        "input": {
          "type": "LocalFillInput",
          "order": "7:1",
          "trade_id": "inventory-buy",
          "quantity": 1,
          "price": 499
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_revision": 3,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 1008,
        "peer_available": 998992,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              3,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "inventory-buy",
              "channel": 0,
              "source": "localsim.trades"
            },
            "core_sequence": 6,
            "ingress": 6,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|6|2|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|10|0|0|0|3|0|0|0|mark:500:1|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:3:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"inventory-buy\":1:1:499:0|fund:1:0:900:0|fund:2:0:100:2|fund:3:0:0:6|budget:1008:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:7:0:1:0:4:0"
      },
      {
        "ordinal": 3,
        "at": 3,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 2,
          "received_at": 3,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_positions": [
          10,
          0,
          0,
          0
        ],
        "peer_revision": 3,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 1008,
        "peer_available": 998992,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 2
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              3,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 7,
            "ingress": 7,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|7|3|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|10|0|0|0|3|0|0|0|mark:500:3|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:3:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"inventory-buy\":1:1:499:0|fund:1:0:900:0|fund:2:0:100:2|fund:3:0:0:6|budget:1008:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:8:0:1:0:4:0"
      },
      {
        "ordinal": 4,
        "at": 4,
        "input": {
          "type": "LocalFillInput",
          "order": "7:2",
          "trade_id": "inventory-close",
          "quantity": 1,
          "price": 501
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 4,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 1008,
        "peer_available": 998992,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              1,
              0,
              2,
              0,
              0
            ],
            "execution": 2
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:2",
              "quantity": 1,
              "price": 501,
              "trade_id": "inventory-close",
              "channel": 0,
              "source": "localsim.trades"
            },
            "core_sequence": 8,
            "ingress": 8,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|8|4|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|9|0|0|0|2|0|0|0|mark:500:3|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:3:1:0:-1:0:0:0:1:-1:2:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"inventory-buy\":1:1:499:0|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"inventory-close\":2:1:501:0|fund:1:0:900:0|fund:2:0:100:2|fund:3:0:0:6|budget:1008:0:0:0|quarantine:0:0|admitted:1|admitted:1|ids:3:0:9:0:1:0:4:0"
      },
      {
        "ordinal": 5,
        "at": 5,
        "input": {
          "type": "MarketPulse",
          "source": "fixture.book",
          "epoch": 1,
          "sequence": 3,
          "received_at": 5,
          "bid": 499,
          "ask": 501,
          "bid_quantity": 10,
          "ask_quantity": 10,
          "healthy": true,
          "subscription_generation": 1,
          "session_second": 32460
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 7,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 1112,
        "peer_available": 998888,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": {
          "error": "None",
          "withdraw": false,
          "generation": 3
        },
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              1,
              0,
              0,
              2,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:3",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:4",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              3,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "ReferenceMark",
              "price": 500
            },
            "core_sequence": 9,
            "ingress": 9,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:2"
            },
            "core_sequence": 10,
            "ingress": 10,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:2",
              "cancel_state": 2
            },
            "core_sequence": 11,
            "ingress": 11,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:2",
              "terminal_kind": 1,
              "exact_quantity": 1
            },
            "core_sequence": 12,
            "ingress": 12,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 0,
                "offset": 0,
                "price": 499,
                "quantity": 1
              }
            },
            "core_sequence": 13,
            "ingress": 13,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:3"
            },
            "core_sequence": 14,
            "ingress": 14,
            "error": "None"
          },
          {
            "input": {
              "type": "SubmitOrder",
              "intent": {
                "account": "fixture.account",
                "instrument": "rb_FIXTURE",
                "day": 20261105,
                "side": 1,
                "offset": 1,
                "price": 501,
                "quantity": 3
              }
            },
            "core_sequence": 15,
            "ingress": 15,
            "error": "None"
          },
          {
            "input": {
              "type": "WorkingReport",
              "order": "7:4"
            },
            "core_sequence": 16,
            "ingress": 16,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|16|5|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|9|0|0|0|3|0|0|0|mark:500:5|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:3:1:1:1:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:3:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"inventory-buy\":1:1:499:0|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"inventory-close\":2:1:501:0|fund:1:0:900:0|fund:2:0:100:2|fund:3:0:0:2|fund:4:100:0:2|fund:5:0:0:6|budget:1112:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:5|admitted:5|ids:5:0:17:0:2:0:6:0"
      },
      {
        "ordinal": 6,
        "at": 6,
        "input": {
          "type": "LocalTradeQueryInput",
          "query_instance": 7,
          "query_serial": 100,
          "omit_last_trade": false,
          "bypass_backpressure_for_test": false
        },
        "error": "None",
        "completed": true,
        "mode": 0,
        "core_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 7,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 1112,
        "peer_available": 998888,
        "cleanup_lane": false,
        "query_aligned": true,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              1,
              0,
              0,
              2,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:3",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              1,
              0,
              0
            ],
            "execution": 1
          },
          {
            "id": "7:4",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              3,
              0,
              0
            ],
            "execution": 1
          }
        ],
        "calls": [
          {
            "input": {
              "type": "TradeReport",
              "order": "7:1",
              "quantity": 1,
              "price": 499,
              "trade_id": "inventory-buy",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 17,
            "ingress": 17,
            "error": "Duplicate"
          },
          {
            "input": {
              "type": "TradeReport",
              "order": "7:2",
              "quantity": 1,
              "price": 501,
              "trade_id": "inventory-close",
              "channel": 1,
              "source": "localsim.query.trades"
            },
            "core_sequence": 18,
            "ingress": 18,
            "error": "Duplicate"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|18|6|0|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|9|0|0|0|3|0|0|0|mark:500:5|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:3:1:1:1:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:3:0:0:-1:0:0:0:1:-1:1:0;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"inventory-buy\":1:1:499:0|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"inventory-close\":2:1:501:0|fund:1:0:900:0|fund:2:0:100:2|fund:3:0:0:2|fund:4:100:0:2|fund:5:0:0:6|budget:1112:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:5|admitted:5|ids:5:0:19:0:2:0:6:0"
      },
      {
        "ordinal": 7,
        "at": 7,
        "input": {
          "type": "StopLocalQuotes"
        },
        "error": "None",
        "completed": true,
        "mode": 1,
        "core_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_positions": [
          9,
          0,
          0,
          0
        ],
        "peer_revision": 9,
        "pending_reports": 0,
        "pending_delivery": 0,
        "query_blocked": false,
        "delivery_loss": false,
        "core_required": 1004,
        "peer_available": 998996,
        "cleanup_lane": false,
        "query_aligned": null,
        "decision": null,
        "orders": [
          {
            "id": "7:1",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              1,
              0,
              0,
              0,
              0
            ],
            "execution": 3
          },
          {
            "id": "7:2",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              1,
              0,
              0,
              2,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:3",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 0,
              "offset": 0,
              "price": 499,
              "quantity": 1
            },
            "parts": [
              0,
              0,
              0,
              1,
              0
            ],
            "execution": 4
          },
          {
            "id": "7:4",
            "intent": {
              "account": "fixture.account",
              "instrument": "rb_FIXTURE",
              "day": 20261105,
              "side": 1,
              "offset": 1,
              "price": 501,
              "quantity": 3
            },
            "parts": [
              0,
              0,
              0,
              3,
              0
            ],
            "execution": 4
          }
        ],
        "calls": [
          {
            "input": {
              "type": "KillRequested"
            },
            "core_sequence": 19,
            "ingress": 19,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:3"
            },
            "core_sequence": 20,
            "ingress": 20,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:3",
              "cancel_state": 2
            },
            "core_sequence": 21,
            "ingress": 21,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:3",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 22,
            "ingress": 22,
            "error": "None"
          },
          {
            "input": {
              "type": "RequestCancel",
              "order": "7:4"
            },
            "core_sequence": 23,
            "ingress": 23,
            "error": "None"
          },
          {
            "input": {
              "type": "CancelReport",
              "order": "7:4",
              "cancel_state": 2
            },
            "core_sequence": 24,
            "ingress": 24,
            "error": "None"
          },
          {
            "input": {
              "type": "TerminalReport",
              "order": "7:4",
              "terminal_kind": 1,
              "exact_quantity": 0
            },
            "core_sequence": 25,
            "ingress": 25,
            "error": "None"
          }
        ],
        "canonical": "domain.v6|7|rb_FIXTURE|fixture.contract.v1|fixture.calendar.v1|20261105|1|25|7|1|0|0|scope:\"fixture.account\":\"SHFE\":\"rb_FIXTURE\":20261105:1|strategy:\"fixture.default.strategy\"|trade-mode:1|9|0|0|0|0|0|0|0|mark:500:5|order:7:1:2:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:1:0:1:1:0:0:1:-1:3:0;strategy:\"fixture.default.strategy\"|order:7:2:3:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:3:1:1:1:1:0:0:1:1:4:2;action:1:2;current:1;strategy:\"fixture.default.strategy\"|order:7:3:4:\"fixture.account\":\"rb_FIXTURE\":20261105:0:0:499:1:0:0:0:1:0:0:1:1:4:2;action:2:2;current:2;strategy:\"fixture.default.strategy\"|order:7:4:5:\"fixture.account\":\"rb_FIXTURE\":20261105:1:1:501:3:0:0:0:1:0:0:1:1:4:2;action:3:2;current:3;strategy:\"fixture.default.strategy\"|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"inventory-buy\":1:1:499:0|trade:\"fixture.account\":20261105:\"SHFE\":\"p03.localsim.day.v1\":\"inventory-close\":2:1:501:0|fund:1:0:900:0|fund:2:0:100:2|fund:3:0:0:2|fund:4:0:0:0|fund:5:0:0:0|budget:1004:0:0:0|quarantine:0:0|admitted:1|admitted:1|admitted:5|admitted:5|ids:5:0:26:0:4:0:6:0"
      }
    ],
    "provenance": {
      "run": "2026-11-08-market-state-debug",
      "configuration": "Debug",
      "sha256": "e51391fa206e13b4da628527ce7a5ee26734d8cf85b4ff0d20b03610a8ab61b3",
      "producer": "p03_export_quote_runner"
    }
  }
};
