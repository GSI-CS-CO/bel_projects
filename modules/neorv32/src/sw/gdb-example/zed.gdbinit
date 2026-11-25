set architecture riscv:rv32

define target hookpost-remote
  monitor reset halt
  load
end
