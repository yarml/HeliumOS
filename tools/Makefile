.PHONY: all upconfig

all: upconfig

upconfig: upconfig/Cargo.toml $(wildcard upconig/src/*.rs)
	cd upconfig && cargo build --release
	cp upconfig/target/release/upconfig ../sysroots/build/bin/upconfig

clean:
	cd upconfig && cargo clean
