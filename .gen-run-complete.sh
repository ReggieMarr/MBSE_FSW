#!/bin/bash

_run_sh_completions()
{
  local cur prev opts
  COMPREPLY=()
  cur="${COMP_WORDS[COMP_CWORD]}"
  prev="${COMP_WORDS[COMP_CWORD-1]}"
  opts="docker-build build inspect exec gds gen-deps teardown update sync test topology --daemon --persist --debug --as-host --local --clean --host-thread-ctl --help --force"

  if [[ ${cur} == -* ]]; then
    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
    return 0
  fi

  case "${prev}" in
    inspect)
      # You can add container names here if you want to provide completion for specific containers
      local containers="fsw gds"
      COMPREPLY=( $(compgen -W "${containers}" -- ${cur}) )
      return 0
      ;;
    *)
      COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
      return 0
      ;;
  esac
}

complete -F _run_sh_completions run.sh
