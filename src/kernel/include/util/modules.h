
#ifndef MODULES_H
#define MODULES_H

extern int TTY_MODULE;

#define IS_INITIALIZED(module) (module)
#define INITIALIZED(module) do {module = 1;} while (0)

#endif // MODULES_H