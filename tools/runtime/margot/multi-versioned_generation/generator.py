import argparse
import json
import os

parser = argparse.ArgumentParser(
    description="Generate the margot configuration and execution file from metadata."
)
parser.add_argument("problem_json", help="The path to the problem metadata")
parser.add_argument("--agora", help="The path to the agora metadata (NO AGORA BY DEFAULT)")

args = parser.parse_args()
with open(args.problem_json,) as problem_file:
    problem_json = json.load(problem_file)
with open("version.json",) as version_file:
    version_json = json.load(version_file)

margot_json = {
    "name": problem_json["name"],
    "version": "Generated_v1",
    "blocks":
    [
        {
            "name": "default_block",
            "monitors":
            [
                {
                    "name": "time_monitor",
                    "type": "time",
                    "log": ["average"],
                    "constructor": [ "margot::TimeUnit::MICROSECONDS", 1]
                },
                {
                    "name": "configuration_error",
                    "type": "int",
                    "log": ["average"],
                    "constructor": [1],
                    "stop": [{"config_error_val": "int"}]
                }
            ],
            "knobs":
            [
                {
                    "name": "version",
                    "type": "int",
                    "range": [0, len(version_json["versions"]), 1]
                }
            ],
            "metrics":
			[
				{
					"name": "timing",
					"type": "double",
					"distribution": "yes",
					"observed_by": "time_monitor",
					"reactive_inertia": 0
				},
                {
                    "name": "config_error",
                    "type": "double",
                    "distribution": "no",
                    "observed_by": "configuration_error",
                    "reactive_inertia": 0
                }
			],
            "extra-functional_requirements":
            [
                {
                    "name": "default",
                    "minimize":
                    {
                        "geometric_mean":
                        [
                            {"timing": 1}
                        ]
                    },
                    "subject_to":
                    [
                        {
                            "subject": "config_error",
                            "comparison": "le",
                            "value": 0,
                            "confidence": 0
                        }
                    ]
                }
            ]
        }
    ]
}

if args.agora:
    for metric in margot_json["blocks"][0]["metrics"]:
        metric["prediction_plugin"] = "model"
        metric["prediction_parameters"] = [{"quality_threshold": "{'r2':0.8}"}]
    agora_file = open(args.agora,)
    agora_json = json.load(agora_file)
    margot_json["blocks"][0]["agora"] = agora_json

features = []
features.extend(problem_json["feature"])

version_conditioned = list(filter(lambda x: "condition" in x, version_json["versions"]))
for version in version_conditioned:
    features.append(
        {
            "name": version["name"]+"_condition",
            "type": version["condition"]["type"],
            "comparison": "="
        })

if features != []:
    margot_json["blocks"][0]["feature_distance"] = "euclidean"
    margot_json["blocks"][0]["features"] = features

with open("margot.json", "w") as f:
    json.dump(margot_json, f, ensure_ascii=False, indent=4)

with open(problem_json["name"]+".cpp", "w") as file, open(problem_json["name"]+".h", "w") as header:
    file.writelines([
        "#include <margot/margot.hpp>\n",
        "#include <chrono>\n",
        "#include <thread>\n",
        "#include <list>\n",
        "#include <string>\n",
        "#include <iostream>\n",
        "#include <cstdio>\n",
        "extern \"C\" {\n",
        "#include \"" + problem_json["name"] + ".h\"\n",
        "}\n"
    ])
    header.writelines([
        "#ifndef " + problem_json["name"].upper() + "_H\n",
        "#define " + problem_json["name"].upper() + "_H\n",
    ])
    
    if "header" in problem_json:
        header.writelines([problem_json["header"] + "\n"])

    for version in version_json["versions"]:
        file.writelines([
            "extern \"C\" {\n",
            "#include \"" + version["name"] + ".h\"\n",
            "}\n"])
        #header.writelines([version["header"] + "\n"])
    
    #file.writelines(["namespace " + problem_json["name"] + "_kernel {\n"])
    #header.writelines([
    #    "namespace " + problem_json["name"] + "_kernel\n",
    #    "{\n"
    #])

    file.writelines([
        "typedef struct\n",
        "{\n",
    ])

    for feature in problem_json["feature"]:
        file.writelines(["    " + feature["type"] + " " + feature["name"] + ";\n"])
    for version in version_conditioned:
        file.writelines(["    " + version["condition"]["type"] + " " + version["name"] + "_condition;\n"])

    file.writelines(["} features;\n"])

    return_values = list(filter(lambda x: x["name"] == "return", problem_json["output"]))
    if return_values == []:
        return_value = "void"
    else:
        return_value = return_values[0]["type"]

    output_parameters = list(filter(lambda x: x["name"] != "return", problem_json["output"]))
    function_signature = return_value + " " + problem_json["name"] + "("
    function_signature += ", ".join(
        list(
            map(lambda x: x["type"] + " " + x["name"], problem_json["input"])
        ) +
        list(
            map(lambda x: x["type"] + "* " + x["name"], output_parameters)
        )
    )
    function_signature += ")"
    file.writelines([
        function_signature + "\n",
        "{\n"
    ])
    header.writelines(["" + function_signature + ";\n"])
    header.writelines([
        "bool check_dse();\n"
    ])
    file.writelines([
        "    margot::init();\n",
    ])

    pre_signature = "void (*pre) ("
    pre_signature += ", ".join(list(map(lambda x: x["type"], problem_json["input"])))
    pre_signature += ")"
    launch_signature = "void (*launch) ()"
    post_signature = return_value + " (*post) ("
    post_signature += ", ".join(list(map(lambda x: x["type"] + "&", output_parameters)))
    post_signature += ")"

    file.writelines([
        "    typedef " + pre_signature + ";\n",
        "    typedef " + launch_signature + ";\n",
        "    typedef "+ post_signature + ";\n",
        "    pre pre_fun[" + str(len(version_json["versions"])) + "];\n",
        "    launch launch_fun[" + str(len(version_json["versions"])) + "];\n",
        "    post post_fun[" + str(len(version_json["versions"])) + "];\n",
        "    char* names[" + str(len(version_json["versions"])) + "];\n"
    ])

    i = 0
    for version in version_json["versions"]:
        file.writelines([
            "    pre_fun[" + str(i) + "] = " + version["name"] + "_pre_execution;\n",
            "    launch_fun[" + str(i) + "] = " + version["name"] + "_launch;\n",
            "    post_fun[" + str(i) + "] = " + version["name"] + "_post_execution;\n",
            "    names[" + str(i) + "] = static_cast<char*>(\"" + version["name"] + "\");\n"
        ])
        i += 1
    
    file.writelines([
        "    int version = 0;\n",
        "    features feat;\n"
    ])

    for feature in problem_json["feature"]:
        file.writelines(["    feat." + feature["name"] + " = " + feature["function"] + ";\n"])
    for version in version_conditioned:
        file.writelines(["    feat." + version["name"] + "_condition = " + version["condition"]["function"] +";\n"])

    file.writelines(["    margot::default_block::context().manager.wait_for_knowledge(5);\n"])
    update_params = []
    for feature in problem_json["feature"]:
        update_params.append("feat." + feature["name"])
    for version in version_conditioned:
        update_params.append("feat." + version["name"] + "_condition")
    update_params.append("version")
    file.writelines([
        "    if(margot::default_block::update(" + ", ".join(update_params) + "))\n",
        "    {\n",
        "        margot::default_block::context().manager.configuration_applied();\n",
        "        std::cout << \"< < < CHANGED VERSION > > >\" << std::endl;\n",
        "    }\n"
    ])
    file.writelines([
        "    int error = 0;\n"
        "    switch(version)\n",
        "    {\n"
    ])
    for version in version_conditioned:
        file.writelines([
            "        case " + str(version_json["versions"].index(version)) + ":\n",
            "        if(!(" + version["condition"]["constraint"] + "(feat." + version["name"] + "_condition)))\n",
            "        {\n",
            "            error = 1;\n",
            "            version = 0;\n",
            "        }\n",
            "        break;\n"
        ])

    file.writelines([
        "        default:\n",
        "        break;\n",
        "    }\n"
    ])

    file.writelines([
        "    bool done = false;\n",
        "    while(!done)\n",
        "    {\n",
        "        try\n",
        "        {\n",
        "            pre_fun[version](" + ", ".join(list(map(lambda x: x["name"], problem_json["input"]))) + ");\n",
        "            margot::default_block::start_monitors();\n",
        "            launch_fun[version]();\n",
        "            margot::default_block::stop_monitors();\n",
        "            margot::default_block::push_custom_monitor_values(error);\n",
    ])

    if return_value != "void":
        file.writelines([
            "            " + return_value + " output = post_fun[version](" + ", ".join(list(map(lambda x: "*" + x["name"], output_parameters))) + ");\n",
            "            margot::default_block::log();\n",
            "            return output;\n",
        ])
    else:
        file.writelines([
            "            post_fun[version](" + ", ".join(list(map(lambda x: "*" + x["name"], output_parameters))) + ");\n"
            "            margot::default_block::log();\n"
            "            done = true;\n",
        ])
    file.writelines([
        "        }\n",
        "        catch(...)\n",
        "        {\n",
        "            if(!version)\n",
        "                throw;\n",
        "            version = 0;\n",
        "            error = 2;\n",
        "        }\n"
        "    }\n"
    ])

    file.writelines([
        "}\n",
        "bool check_dse()\n",
        "{\n",
        "    return margot::default_block::context().manager.in_design_space_exploration();\n",
        "}\n",
#        "}\n"
    ])
    
    header.writelines([
        #"}\n",
        #"using namespace " + problem_json["name"] + "_kernel;\n"
        "#endif"
    ])

with open("CMakeLists.txt", "w") as cmake_file:
    cmake_file.writelines([
        "cmake_minimum_required(VERSION 3.10)\n\n",
        "project(" + problem_json["name"] + ")\n\n",
        "find_package (margot_heel_generator REQUIRED)\n",
        "set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -fPIC\")\n"  #-D_GLIBCXX_USE_CXX11_ABI=0
        "margot_heel_generate_interface(\"${CMAKE_CURRENT_SOURCE_DIR}/margot.json\")\n\n"
    ])

    for version in version_json["versions"]:
        cmake_file.writelines([
            "add_subdirectory(" + version["folder"] + ")\n" 
        ])
    cmake_file.writelines(["\n"])

    cmake_file.writelines([
        "set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)\n",
        "set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)\n",
        "set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)\n\n",

        "set(CMAKE_SKIP_BUILD_RPATH FALSE)\n",
        "set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)\n",
        "set(CMAKE_INSTALL_RPATH \"${CMAKE_INSTALL_PREFIX}/lib\")\n",
        "set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)\n\n",

        "list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES \"${CMAKE_INSTALL_PREFIX}/lib\" isSystemDir)\n",
        "if(\"${isSystemDir}\" STREQUAL \"-1\")\n",
        "    set(CMAKE_INSTALL_RPATH \"${CMAKE_INSTALL_PREFIX}/lib\")\n",
        "endif(\"${isSystemDir}\" STREQUAL \"-1\")\n\n",

        "add_library(" + problem_json["name"] + " SHARED " + problem_json["name"] + ".h " + problem_json["name"] + ".cpp)\n",
        "add_library(" + problem_json["name"] + "::" + problem_json["name"] + " ALIAS " + problem_json["name"] + ")\n\n",

        "target_include_directories(" + problem_json["name"] + " PUBLIC $<BUILD_INTERFACE:${HEADER_DIR}>)\n",
        "target_link_libraries(" + problem_json["name"] + " PUBLIC $<BUILD_INTERFACE:margot::margot_heel_interface>)\n\n"
    ])

    for version in version_json["versions"]:
        cmake_file.writelines([
            "target_include_directories(" + problem_json["name"] + " PUBLIC \n",
            "    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/" + version["name"] + ">\n",
            #"$<INSTALL_INTERFACE:" + version["name"] + ">\n",
            ")\n",
            "target_link_libraries(" + problem_json["name"] + " PUBLIC $<BUILD_INTERFACE:" + version["name"] + ">)\n\n"
        ])
    cmake_file.writelines([
        "target_compile_features(" + problem_json["name"] + " PUBLIC cxx_std_17)\n",
        "set_target_properties(" + problem_json["name"] + " PROPERTIES PUBLIC_HEADER \"" + problem_json["name"] + ".h\")\n\n",

        "include(CMakePackageConfigHelpers)\n",
        "include(GNUInstallDirs)\n\n",

        "install(TARGETS " + problem_json["name"] + " " + " ".join([x["name"] for x in version_json["versions"]]) + "\n",
        "    EXPORT " + problem_json["name"] + "Targets\n",
        "    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}\n",
        "    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}\n",
        "    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/" + problem_json["name"] + "\n",
        "    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/" + problem_json["name"] + "\n",
        ")\n\n",

        "set(CONFIG_INSTALL_PATH \"${CMAKE_INSTALL_LIBDIR}/cmake\")\n",
        "write_basic_package_version_file(\n",
        "    " + problem_json["name"] + "ConfigVersion.cmake\n",
        "    VERSION \"1.0\"\n",
        "    COMPATIBILITY ExactVersion\n",
        ")\n",
        "install(\n",
        "    EXPORT " + problem_json["name"] + "Targets\n",
        "    FILE " + problem_json["name"] + "Targets.cmake\n",
        "    NAMESPACE " + problem_json["name"] + "::\n",
        "    DESTINATION ${CONFIG_INSTALL_PATH}\n",
        ")\n\n",

        "configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/config.cmake.in " + problem_json["name"] + "Config.cmake @ONLY)\n",
        "install(\n",
        "    FILES ${CMAKE_CURRENT_BINARY_DIR}/" + problem_json["name"] + "Config.cmake\n",
        "    ${CMAKE_CURRENT_BINARY_DIR}/" + problem_json["name"] + "ConfigVersion.cmake\n",
        "    DESTINATION ${CONFIG_INSTALL_PATH}\n",
        ")\n"
    ])

if not os.path.exists("./config"):
    os.makedirs("./config")

with open("./config/config.cmake.in", "w") as config_file:
    config_file.writelines([
        "@PACKAGE_INIT@\n",
        "include(CMakeFindDependencyMacro)\n",
        "include(GNUInstallDirs)\n",
        "find_dependency(margot_heel_generator REQUIRED)\n"
        #ADD XRT DEPENDENCY
        "include(\"${CMAKE_CURRENT_LIST_DIR}/" + problem_json["name"] + "Targets.cmake\")\n"
    ])