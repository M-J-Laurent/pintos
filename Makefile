start:
	@echo "cd into pintos"
	@docker run -it --rm --name pintos -v "$(CURDIR):/home/PKUOS/pintos" pkuflyingpig/pintos bash
boot:
	@echo "Building boot sector"
	cd ./src/threads && make clean && make
	cd ./src/threads/build && pintos --
userprog:
	@echo "Building user program"
	cd ./src/userprog && make