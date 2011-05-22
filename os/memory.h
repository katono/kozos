#ifndef KOZOS_MEMORY_H_INCLUDED
#define KOZOS_MEMORY_H_INCLUDED

int kzmem_init(void);
void *kzmem_alloc(int size);
void kzmem_free(void *mem);

#endif /* KOZOS_MEMORY_H_INCLUDED */
