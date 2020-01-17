#!/usr/bin/env python3
"""
This script builds a simple JSON map: MeSH ID => MeSH term
The primary name is used as term.
"""
import json
import argparse
import gzip

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("desc")
    parser.add_argument("suppl")
    parser.add_argument("output")
    args = parser.parse_args()
    result = {}
    # Process desc
    with gzip.open(args.desc, "r") as descfile:
        for line in descfile:
            obj = json.loads(line)
            result[obj["id"]] = obj["name"]
    # Process suppl
    with gzip.open(args.suppl, "r") as supplfile:
        for line in supplfile:
            obj = json.loads(line)
            result[obj["id"]] = obj["name"]
    # Outfile
    with open(args.output, "w") as outfile:
        json.dump(result, outfile)