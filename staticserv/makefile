CC := gcc

FLAGS := -Wvla -Wfloat-conversion -Wfloat-equal -I ./inc/ -lm

OBJS_APP := ./out/fdsarr.o ./out/httptools.o ./out/pathtools.o ./out/logger.o ./out/main.o

.PHONY: release debug clean

.NOTPARALLEL: release debug clean

./out/:
	mkdir out

release: ./out/lastrelease | app.exe

debug: FLAGS += -g3
debug: ./out/lastdebug | app.exe

./out/lastrelease: ./out/
	touch ./out/lastrelease
	-rm app.exe
	-rm ./out/lastdebug

./out/lastdebug: ./out/
	touch ./out/lastdebug
	-rm app.exe
	-rm ./out/lastrelease

app.exe: ./out/ | $(OBJS_APP)
	$(CC) $(OBJS_APP) -o $@ -lm

./out/%.o: ./src/%.c ./inc/*.h
	$(CC) $(FLAGS) -c $< -o $@

clean:
	-rm -r out
	-rm app.exe
