import argparse
import json
import os

parser = argparse.ArgumentParser(
    description="Generate the header and version.json file for a given version"
)
parser.add_argument("problem_json", help="Location of the problem.json file")
parser.add_argument("name", help="The name of the version")
parser.add_argument("-f", action="store_true", help="Overwrites the version if present")

args = parser.parse_args()

if not os.path.exists("./" + args.name):
    os.makedirs("./" + args.name)
else:
    if(args.f):
        if os.path.isfile("./" + args.name + "/version.json"):
            os.remove("./" + args.name + "/version.json")
        if os.path.isfile("./" + args.name + "/" + args.name + ".h"):
            os.remove("./" + args.name + "/" + args.name + ".h")
    else:
        print("Version " + args.name + " already exists, use -f if you want to overwrite it.")
        print("Terminating.")
        exit()

version_json = {
    "name": args.name,
    "folder": args.name,
    "condition":
    {
        "function": args.name + "_condition()",
        "type": "int",
        "constraint": "[](int i) -> bool {return i != 0;}"
    }
}

with open("./" + args.name + "/version.json", "w") as json_file:
    json.dump(version_json, json_file, ensure_ascii=False, indent=4)

with open(args.problem_json,) as problem_file:
    problem_json = json.load(problem_file)

with open("./" + args.name + "/" + args.name + ".h", "w") as header_file:
    header_file.writelines([
        "#ifndef " + (args.name + "_hpp").upper() + "\n",
        "#define " + (args.name + "_hpp").upper() + "\n\n",

#        "#include <cstdio>\n",
        problem_json["header"] + "\n\n",

#        "namespace " + args.name + "\n",
#        "{\n",
    ])

    input_params = []
    for i in problem_json["input"]:
        input_params.append(i["type"] + " " + i["name"])

    header_file.writelines([
        "void " + args.name + "_pre_execution(" + ", ".join(input_params) + ");\n",
        "void " + args.name + "_launch();\n"
    ])

    output_params = []
    return_params = []
    for o in problem_json["output"]:
        if o["name"] == "return":
            return_params.append(o["type"])
        else:
            output_params.append(o["type"] + "& " + o["name"])

    if len(return_params) == 0:
        return_params.append("void")
        
    header_file.writelines([
        return_params[0] + " " + args.name + "_post_execution(" + ", ".join(output_params) + ");\n",
        "int " + args.name + "_condition();\n",

#        "    //These functions are not called by the margot files but should be used in the definition of the launch function\n",
#        "    void data_in(bool ping);\n",
#        "    void data_in_done(bool ping);\n",
#        "    void execute(int ping);\n",
#        "    void execute_done(bool ping);\n",
#        "    void data_out(bool ping);\n",
#        "    void data_out_done(bool ping);\n"
#        "}\n\n",

        "#endif"
    ])
