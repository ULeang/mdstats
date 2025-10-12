EXEC:=mdstats.exe

.SHELL: pwsh

.PHONY: configure
configure:
	@mkdir -p build
	@cmake -DCMAKE_BUILD_TYPE:STRING=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
		-DCMAKE_C_COMPILER=gcc \
		-DCMAKE_CXX_COMPILER=g++ \
		-S . \
		-B build \
		-G "MinGW Makefiles"

.PHONY: build
build:
	@cmake --build build

.PHONY: install
install:
	@cmake --install build --prefix install
	@windeployqt install/$(EXEC)

.PHONY: debug
debug :
	@mkdir -p build
	@cmake -DCMAKE_BUILD_TYPE:STRING=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
		-DCMAKE_C_COMPILER=gcc \
		-DCMAKE_CXX_COMPILER=g++ \
		-S . \
		-B build \
		-G "MinGW Makefiles"
	@cmake --build build
	@make run

.PHONY: release
release :
	@mkdir -p build
	@cmake -DCMAKE_BUILD_TYPE:STRING=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
		-DCMAKE_C_COMPILER=gcc \
		-DCMAKE_CXX_COMPILER=g++ \
		-S . \
		-B build \
		-G "MinGW Makefiles"
	@cmake --build build
	@make run

.PHONY: run
run: export PATH:=./lib;${PATH}
run: ./build/$(EXEC)
	@./build/$(EXEC)

./build/$(EXEC):
	@make debug

.PHONY: clean
clean:
	@rm -rf build install