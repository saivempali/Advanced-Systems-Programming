programs: combiner mapper reducer

combiner: combiner.c
	gcc -o combiner combiner.c

mapper: mapper.c
	gcc -o mapper mapper.c

reducer: reducer.c
	gcc -o reducer reducer.c
	@echo Compilation Successful!

clean: 
	@rm combiner mapper reducer
	@echo All programs are cleaned. Compile again.
