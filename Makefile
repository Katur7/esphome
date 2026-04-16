CONFIG ?= einkframe.yaml
DOCKER = docker compose run --rm esphome

.PHONY: validate compile upload logs run dashboard clean

validate:
	$(DOCKER) config $(CONFIG)

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
