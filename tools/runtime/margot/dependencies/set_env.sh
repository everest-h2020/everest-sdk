
export CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}":\
${EVEREST_PWD}/tools/runtime/margot/dependencies/paho.mqtt.c/install/lib/cmake/eclipse-paho-mqtt-c\
:\
${EVEREST_PWD}/tools/runtime/margot/dependencies/json/install/lib/cmake/nlohmann_json\
"
export OPTIONS_CMAKE=${OPTIONS_CMAKE}" -DBoost_NO_BOOST_CMAKE=TRUE \
-DBoost_NO_SYSTEM_PATHS=TRUE \
-DBOOST_ROOT:PATHNAME=${EVEREST_PWD}/tools/runtime/margot/dependencies/boost_1_60_0/install \
-DBoost_LIBRARY_DIRS:FILEPATH=${EVEREST_PWD}/tools/runtime/margot/dependencies/boost_1_60_0/install/lib \
"
