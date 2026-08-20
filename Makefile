CONFIG ?= einkframe.yaml
DEVICE ?=
DEVICE_FLAG = $(if $(DEVICE),--device $(DEVICE),)

# USB=1 switches to the privileged service that can see /dev, for flashing a
# board over USB. Default service has no device access — OTA only.
SERVICE = $(if $(USB),esphome-usb,esphome)
DOCKER = docker compose run --rm $(SERVICE)

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
	$(DOCKER) upload $(CONFIG) $(DEVICE_FLAG)

logs:
	$(DOCKER) logs $(CONFIG) $(DEVICE_FLAG)

run:
	$(DOCKER) run $(CONFIG) $(DEVICE_FLAG)

dashboard:
	docker run --rm -it -p 6052:6052 -v "$(PWD)/config:/config" ghcr.io/esphome/esphome dashboard /config

clean:
	$(DOCKER) clean $(CONFIG)
