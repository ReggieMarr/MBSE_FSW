#!/bin/bash

_run_sh_completions()
{
  local cur prev opts
  COMPREPLY=()
  cur="${COMP_WORDS[COMP_CWORD]}"
  prev="${COMP_WORDS[COMP_CWORD-1]}"
  opts="docker-build build inspect exec gds gen-deps teardown --daemon --persist --debug --as-host --local --clean --host-thread-ctl --help"

  if [[ ${cur} == -* ]]; then
    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
    return 0
  fi

  COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
  return 0
}

complete -F _run_sh_completions run.sh
