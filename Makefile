PROJ:=mdstats
EXEC:=mdstats_with_console

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

.PHONY: toast
toast:
	@cd module/toast_notify && cargo build --release

.PHONY: build
build:
	@cmake --build build --parallel 16

.PHONY: install
install:
	@cmake --install build --prefix $(PROJ)
	@rm $(PROJ)/libmodstatstable.dll.a
	@windeployqt $(PROJ)/$(EXEC).exe

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
	@cmake --build build --parallel 16

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
	@cmake --build build --parallel 16

.PHONY: release_console
release_console:
	@mkdir -p build
	@cmake -DCMAKE_BUILD_TYPE:STRING=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
		-DCMAKE_C_COMPILER=gcc \
		-DCMAKE_CXX_COMPILER=g++ \
		-S . \
		-B build \
		-G "MinGW Makefiles" \
		-DWITHOUTCONSOLE:BOOL=true
	@cmake --build build --parallel 16

.PHONY: run
run: export PATH:=./lib;${PATH}
run: ./build/$(EXEC).exe
	@./build/$(EXEC).exe

.PHONY: gdb
gdb: ./build/$(EXEC).exe
	@gdb -q --args ./build/$(EXEC).exe

.PHONY: perf
perf: export PATH:=./lib;${PATH}
perf: ./build/mdstats_perf.exe
	@$^

./build/$(EXEC).exe:
	@make debug

.PHONY: zip
zip: install
	@7z a $(PROJ).zip $(PROJ)/

.PHONY: clean
clean:
	@rm -rf build $(PROJ) $(PROJ).zip