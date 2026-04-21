CONFIG ?= einkframe.yaml
DOCKER = docker compose run --rm esphome

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O0 -g
TEST_SRC = test/test_einkframe_utils.cpp config/packages/einkframe/einkframe_utils/einkframe_utils.cpp
TEST_INCLUDES = -Iconfig/packages/einkframe/einkframe_utils -Itest
TEST_BIN = test/test_runner

.PHONY: validate compile upload logs run dashboard clean test

validate:
	$(DOCKER) config $(CONFIG)

test:
	$(CXX) $(CXXFLAGS) $(TEST_INCLUDES) $(TEST_SRC) -o $(TEST_BIN)
	$(TEST_BIN)

compile:
	$(DOCKER) compile $(CONFIG)

upload:
	$(DOCKER) upload $(CONFIG)

logs:
	$(DOCKER) logs $(CONFIG)

run:
	$(DOCKER) run $(CONFIG)

dashboard:
	$(DOCKER) dashboard /config

clean:
	$(DOCKER) clean $(CONFIG)
