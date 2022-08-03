#include "backend.h"

struct backend const *backend = 0;

void backend_init(struct backend const *be) { backend = be; }
