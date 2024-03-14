import os
import json
import argparse

parser = argparse.ArgumentParser(
    description="It collects the version.json into a single one."
)
parser.add_argument("default_version", help="The name of the folder containing the default version")
args = parser.parse_args()

output = {
    "versions": []
}
all_files = os.listdir(".")
all_subdirectories = [os.path.join(".", args.default_version)]
for i in all_files:
    if os.path.isdir(i) and i != args.default_version:
        all_subdirectories.append(os.path.join(".", i))
for i in all_subdirectories:
    to_be_checked = os.path.join(i, "version.json")
    if os.path.isfile(to_be_checked):
        with open(to_be_checked,) as read_file:
            version_json = json.load(read_file)
            output["versions"].append(version_json)
with open("version.json", "w") as output_file:
    json.dump(output, output_file, ensure_ascii=False, indent=4)