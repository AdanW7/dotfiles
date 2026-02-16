#!/usr/bin/env bash

COMMAND_NAMES=(
  "Recursively Deinitialize SubModules"
  "Recursively Initialize SubModules"
)

COMMANDS=(
  "git submodule deinit --all --force"
  "git submodule update --init --recursive"
)

ALWAYS_PASS=(
  false
  false
)

VALID_EXIT_CODES=(
  "0"
  "0"
)

for i in "${!COMMANDS[@]}"; do
  echo "================================"
  echo "${COMMAND_NAMES[$i]}"
  echo "================================"

  eval "${COMMANDS[$i]}"
  EXIT_CODE=$?

  PASSED=false

  if [[ "${ALWAYS_PASS[$i]}" == "true" ]]; then
    PASSED=true
  else
    for code in ${VALID_EXIT_CODES[$i]}; do
      if [[ "$EXIT_CODE" == "$code" ]]; then
        PASSED=true
        break
      fi
    done
  fi

  if [[ "$PASSED" == "true" ]]; then
    echo -e "${COMMAND_NAMES[$i]}: \e[32mPASSED\e[0m"
  else
    echo -e "${COMMAND_NAMES[$i]}: \e[31mFAILED\e[0m"
    break
  fi
done
