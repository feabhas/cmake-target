// feabhOS_interrupts.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_INTERRUPTS_H
#define FEABHOS_INTERRUPTS_H

#ifdef __cplusplus
extern "C" {
#endif

void feabhOS_enable_interrupts(void);
void feabhOS_disable_interrupts(void);

#ifdef __cplusplus
}
#endif

#endif /* FEABHOS_INTERRUPTS */
