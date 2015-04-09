/* MOTATE addition: support pin change interrupts */
PROVIDE(motate_pin_change_interrupts_start = .);
KEEP( *(.motate.pin_change_interrupts));
PROVIDE(motate_pin_change_interrupts_end = .);
