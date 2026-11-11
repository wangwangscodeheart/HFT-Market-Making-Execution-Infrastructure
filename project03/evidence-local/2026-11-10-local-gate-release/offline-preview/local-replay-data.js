window.P03_LOCAL_REPLAY = {
  "schema": "p03.local-replay.v1",
  "scenario": "delayed-fill-cancel",
  "origin": "LocalSimulator",
  "historical_trace": false,
  "time_unit": "logical_step",
  "blocked_queries": 0,
  "passed": true,
  "frames": [
    {
      "action": "peer_accepts",
      "at": 1,
      "peer_revision": 1,
      "report_id": 0,
      "ingress": 0,
      "core_sequence": 0,
      "error": "None",
      "peer_positions": [
        0,
        0,
        0,
        0
      ],
      "core_positions": [
        0,
        0,
        0,
        0
      ],
      "parts": [
        0,
        0,
        10,
        0,
        0
      ],
      "passed": true
    },
    {
      "action": "deliver_working",
      "at": 2,
      "peer_revision": 1,
      "report_id": 1,
      "ingress": 3,
      "core_sequence": 3,
      "error": "None",
      "peer_positions": [
        0,
        0,
        0,
        0
      ],
      "core_positions": [
        0,
        0,
        0,
        0
      ],
      "parts": [
        0,
        0,
        10,
        0,
        0
      ],
      "passed": true
    },
    {
      "action": "peer_fills_report_held",
      "at": 2,
      "peer_revision": 2,
      "report_id": 0,
      "ingress": 0,
      "core_sequence": 0,
      "error": "None",
      "peer_positions": [
        2,
        0,
        0,
        0
      ],
      "core_positions": [
        0,
        0,
        0,
        0
      ],
      "parts": [
        0,
        0,
        10,
        0,
        0
      ],
      "passed": true
    },
    {
      "action": "request_cancel",
      "at": 3,
      "peer_revision": 2,
      "report_id": 0,
      "ingress": 4,
      "core_sequence": 4,
      "error": "None",
      "peer_positions": [
        2,
        0,
        0,
        0
      ],
      "core_positions": [
        0,
        0,
        0,
        0
      ],
      "parts": [
        0,
        0,
        10,
        0,
        0
      ],
      "passed": true
    },
    {
      "action": "peer_cancels_remainder",
      "at": 3,
      "peer_revision": 3,
      "report_id": 0,
      "ingress": 0,
      "core_sequence": 0,
      "error": "None",
      "peer_positions": [
        2,
        0,
        0,
        0
      ],
      "core_positions": [
        0,
        0,
        0,
        0
      ],
      "parts": [
        0,
        0,
        10,
        0,
        0
      ],
      "passed": true
    },
    {
      "action": "deliver_cancel_reply",
      "at": 4,
      "peer_revision": 3,
      "report_id": 3,
      "ingress": 5,
      "core_sequence": 5,
      "error": "None",
      "peer_positions": [
        2,
        0,
        0,
        0
      ],
      "core_positions": [
        0,
        0,
        0,
        0
      ],
      "parts": [
        0,
        0,
        10,
        0,
        0
      ],
      "passed": true
    },
    {
      "action": "deliver_terminal_before_trade",
      "at": 5,
      "peer_revision": 3,
      "report_id": 4,
      "ingress": 6,
      "core_sequence": 6,
      "error": "None",
      "peer_positions": [
        2,
        0,
        0,
        0
      ],
      "core_positions": [
        0,
        0,
        0,
        0
      ],
      "parts": [
        0,
        2,
        0,
        8,
        0
      ],
      "passed": true
    },
    {
      "action": "deliver_delayed_trade",
      "at": 6,
      "peer_revision": 3,
      "report_id": 2,
      "ingress": 7,
      "core_sequence": 7,
      "error": "None",
      "peer_positions": [
        2,
        0,
        0,
        0
      ],
      "core_positions": [
        2,
        0,
        0,
        0
      ],
      "parts": [
        2,
        0,
        0,
        8,
        0
      ],
      "passed": true
    },
    {
      "action": "repeat_same_delivery",
      "at": 7,
      "peer_revision": 3,
      "report_id": 2,
      "ingress": 8,
      "core_sequence": 8,
      "error": "Duplicate",
      "peer_positions": [
        2,
        0,
        0,
        0
      ],
      "core_positions": [
        2,
        0,
        0,
        0
      ],
      "parts": [
        2,
        0,
        0,
        8,
        0
      ],
      "passed": true
    },
    {
      "action": "query_trade_replay",
      "at": 8,
      "peer_revision": 3,
      "report_id": 0,
      "ingress": 9,
      "core_sequence": 9,
      "error": "Duplicate",
      "peer_positions": [
        2,
        0,
        0,
        0
      ],
      "core_positions": [
        2,
        0,
        0,
        0
      ],
      "parts": [
        2,
        0,
        0,
        8,
        0
      ],
      "passed": true
    }
  ],
  "provenance": {
    "run": "2026-11-10-local-gate-release",
    "configuration": "Release",
    "sha256": "52d0864c00d4132b6c1eaf9dea91b6670c4c3d6bfb54293090ba4f52dc0a7af4",
    "producer": "p03_export_local_replay"
  },
  "query_recovery": {
    "schema": "p03.local-replay.v1",
    "scenario": "query-first-cancel",
    "origin": "LocalSimulator",
    "historical_trace": false,
    "time_unit": "logical_step",
    "blocked_queries": 1,
    "passed": true,
    "frames": [
      {
        "action": "peer_accepts",
        "at": 1,
        "peer_revision": 1,
        "report_id": 0,
        "ingress": 0,
        "core_sequence": 0,
        "error": "None",
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "parts": [
          0,
          0,
          10,
          0,
          0
        ],
        "passed": true
      },
      {
        "action": "deliver_working",
        "at": 2,
        "peer_revision": 1,
        "report_id": 1,
        "ingress": 3,
        "core_sequence": 3,
        "error": "None",
        "peer_positions": [
          0,
          0,
          0,
          0
        ],
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "parts": [
          0,
          0,
          10,
          0,
          0
        ],
        "passed": true
      },
      {
        "action": "peer_fills_report_held",
        "at": 2,
        "peer_revision": 2,
        "report_id": 0,
        "ingress": 0,
        "core_sequence": 0,
        "error": "None",
        "peer_positions": [
          2,
          0,
          0,
          0
        ],
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "parts": [
          0,
          0,
          10,
          0,
          0
        ],
        "passed": true
      },
      {
        "action": "request_cancel",
        "at": 3,
        "peer_revision": 2,
        "report_id": 0,
        "ingress": 4,
        "core_sequence": 4,
        "error": "None",
        "peer_positions": [
          2,
          0,
          0,
          0
        ],
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "parts": [
          0,
          0,
          10,
          0,
          0
        ],
        "passed": true
      },
      {
        "action": "peer_cancels_remainder",
        "at": 3,
        "peer_revision": 3,
        "report_id": 0,
        "ingress": 0,
        "core_sequence": 0,
        "error": "None",
        "peer_positions": [
          2,
          0,
          0,
          0
        ],
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "parts": [
          0,
          0,
          10,
          0,
          0
        ],
        "passed": true
      },
      {
        "action": "deliver_cancel_reply",
        "at": 4,
        "peer_revision": 3,
        "report_id": 3,
        "ingress": 5,
        "core_sequence": 5,
        "error": "None",
        "peer_positions": [
          2,
          0,
          0,
          0
        ],
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "parts": [
          0,
          0,
          10,
          0,
          0
        ],
        "passed": true
      },
      {
        "action": "deliver_terminal_before_trade",
        "at": 5,
        "peer_revision": 3,
        "report_id": 4,
        "ingress": 6,
        "core_sequence": 6,
        "error": "None",
        "peer_positions": [
          2,
          0,
          0,
          0
        ],
        "core_positions": [
          0,
          0,
          0,
          0
        ],
        "parts": [
          0,
          2,
          0,
          8,
          0
        ],
        "passed": true
      },
      {
        "action": "query_recovers_missing_trade",
        "at": 6,
        "peer_revision": 3,
        "report_id": 0,
        "ingress": 7,
        "core_sequence": 7,
        "error": "None",
        "peer_positions": [
          2,
          0,
          0,
          0
        ],
        "core_positions": [
          2,
          0,
          0,
          0
        ],
        "parts": [
          2,
          0,
          0,
          8,
          0
        ],
        "passed": true
      },
      {
        "action": "late_push_after_query",
        "at": 7,
        "peer_revision": 3,
        "report_id": 2,
        "ingress": 8,
        "core_sequence": 8,
        "error": "Duplicate",
        "peer_positions": [
          2,
          0,
          0,
          0
        ],
        "core_positions": [
          2,
          0,
          0,
          0
        ],
        "parts": [
          2,
          0,
          0,
          8,
          0
        ],
        "passed": true
      },
      {
        "action": "repeat_query_trade",
        "at": 8,
        "peer_revision": 3,
        "report_id": 0,
        "ingress": 9,
        "core_sequence": 9,
        "error": "Duplicate",
        "peer_positions": [
          2,
          0,
          0,
          0
        ],
        "core_positions": [
          2,
          0,
          0,
          0
        ],
        "parts": [
          2,
          0,
          0,
          8,
          0
        ],
        "passed": true
      }
    ],
    "provenance": {
      "run": "2026-11-10-local-gate-release",
      "configuration": "Release",
      "sha256": "3a2c7f47685f87f1b0f1945d2474c0388991e08defd7ba7c684de4a68d8c2334",
      "producer": "p03_export_local_replay"
    }
  }
};
