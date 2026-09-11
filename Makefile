.PHONY: setup list new build release run render render-release preview clean

EXAMPLE ?= 02_01_ppm
DEBUG_BINARY = build/debug/bin/$(EXAMPLE)
RELEASE_BINARY = build/release/bin/$(EXAMPLE)
OUTPUT_DIR = output/$(EXAMPLE)
PPM = $(OUTPUT_DIR)/image.ppm
PNG = $(OUTPUT_DIR)/image.png
ARGS ?=
INTERACTIVE = $(filter progressive_viewer room_%,$(EXAMPLE))

setup:
	cmake --preset debug

list:
	@find examples -mindepth 2 -maxdepth 2 -name main.cc -print | sed -E 's|examples/([^/]+)/main.cc|\1|' | sort

new:
	@test -n "$(NAME)" || (echo "Usage: make new NAME=<new_name> FROM=<existing_name>"; exit 1)
	@test -n "$(FROM)" || (echo "Usage: make new NAME=<new_name> FROM=<existing_name>"; exit 1)
	@echo "$(NAME)" | grep -Eq '^[A-Za-z0-9_]+$$' || (echo "NAME may contain only letters, numbers, and underscores"; exit 1)
	@echo "$(FROM)" | grep -Eq '^[A-Za-z0-9_]+$$' || (echo "FROM may contain only letters, numbers, and underscores"; exit 1)
	@test -d "examples/$(FROM)" || (echo "Unknown source example: $(FROM)"; exit 1)
	@test ! -e "examples/$(NAME)" || (echo "Example already exists: $(NAME)"; exit 1)
	cp -R "examples/$(FROM)" "examples/$(NAME)"
	@echo "Created examples/$(NAME) from examples/$(FROM)"

build:
	cmake --preset debug
	cmake --build --preset debug --target $(EXAMPLE)

release:
	cmake --preset release
	cmake --build --preset release --target $(EXAMPLE)

run: build
	$(DEBUG_BINARY) $(ARGS)

ifneq ($(INTERACTIVE),)
render: build
	$(DEBUG_BINARY) $(ARGS)

render-release: release
	$(RELEASE_BINARY) $(ARGS)

preview: render
else
render: build
	@mkdir -p $(OUTPUT_DIR)
	$(DEBUG_BINARY) > $(PPM)
	magick identify $(PPM)
	magick $(PPM) $(PNG)
	@echo "Rendered: $(PPM) and $(PNG)"

render-release: release
	@mkdir -p $(OUTPUT_DIR)
	$(RELEASE_BINARY) > $(PPM)
	magick identify $(PPM)
	magick $(PPM) $(PNG)
	@echo "Rendered (Release): $(PPM) and $(PNG)"

preview: render
	open $(PNG)
endif

clean:
	cmake -E remove_directory build
	cmake -E remove_directory output
