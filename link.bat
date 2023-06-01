@echo Linking Files w/Library
g++ -o app main.o glad.o backend.o -L. -lgmp -lglfw3 -lopengl32 -ld3dcompiler -lgdi32
@echo Linking Complete
.\app