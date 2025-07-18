import json
import os
from datetime import datetime

import garminconnect
from flask import Flask
from flask import jsonify
from flask import request

app = Flask(__name__)


@app.route("/")
def index():
    return "<h2>Available<h2>"


@app.route("/add_weight", methods=["POST"])
def add_weight():
    API_key = os.environ.get("API_KEY")
    key = request.headers.get("authentication")
    print(request.headers)

    if key != API_key:
        print(f"key: {key}")
        print(f"API_key: {API_key}")
        return "no authentication"

    data = request.json
    email = os.environ.get("GARMIN_EMAIL")
    password = os.environ.get("GARMIN_PW")
    garmin = garminconnect.Garmin(email, password)
    garmin.login()

    weight = data["weight"]
    unit = "kg"
    print(f"weight: {weight}")

    # timestamp_str = data["datetime"]
    # try:
    #     # Handle "Z" suffix (UTC)
    #     if timestamp_str.endswith("Z"):
    #         dt = datetime.strptime(timestamp_str, "%Y-%m-%dT%H:%M:%SZ").replace(
    #             tzinfo=timezone.utc
    #         )
    #     else:
    #         dt = datetime.fromisoformat(timestamp_str)
    # except Exception as e:
    #     print(e)
    #     dt = datetime.now()
    #     # return "Fail"

    # # Optional: convert to local time if needed
    # dt_local = dt.astimezone()

    try:
        now = datetime.now().astimezone().isoformat()
        # print(f"now: {now}", flush=True)
        # print(f"dt: {dt_local.isoformat()}", flush=True)
        # add_weight = garmin.add_weigh_in(weight=weight, unitKey=unit, timestamp=dt_local.isoformat(timespec="microseconds"))
        add_weight = garmin.add_weigh_in(weight=weight, unitKey=unit, timestamp=now)
        display_json(f"api.add_weigh_in(weight={weight}, unitKey={unit})", add_weight)
        return jsonify({"status": "success"}), 200  # <-- Explicit success

    except Exception as e:
        print(e)
        return jsonify({"status": "fail", "error": str(e)}), 500  # <-- Explicit failure


@app.route("/get_weight", methods=["GET"])
def get_weight():
    API_key = os.environ.get("API_KEY")
    key = request.headers.get("authentication")
    print(request.headers)

    if key != API_key:
        return "no authentication"

    try:
        email = os.environ.get("GARMIN_EMAIL")
        password = os.environ.get("GARMIN_PW")
        garmin = garminconnect.Garmin(email, password)
        garmin.login()

        current_date = datetime.now()
        last_month_date = current_date.replace(year=current_date.month - 1)
        weights = garmin.get_weigh_ins(
            last_month_date.strftime("%Y-%m-%d"), current_date.strftime("%Y-%m-%d")
        )
        print(weights["dailyWeightSummaries"][1]["latestWeight"]["weight"] / 1000.0)
        return str(
            weights["dailyWeightSummaries"][1]["latestWeight"]["weight"] / 1000.0
        )
    except Exception as e:
        print(e)
        return "Fail"


def display_json(api_call, output):
    """Format API output for better readability."""

    dashed = "-" * 20
    header = f"{dashed} {api_call} {dashed}"
    footer = "-" * len(header)

    print(header)

    if isinstance(output, (int, str, dict, list)):
        print(json.dumps(output, indent=4))
    else:
        print(output)

    print(footer)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8443, ssl_context=("certs/cert.pem", "certs/key.pem"))
