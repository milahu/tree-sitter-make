================================================================================
function call arguments
================================================================================
$(call one,two)

--------------------------------------------------------------------------------

(makefile
  (function_call
    (arguments
      (argument
        (word))
      (argument
        (word)))))

================================================================================
function call arguments with whitespace
================================================================================
$(call one, two, three ,four)

--------------------------------------------------------------------------------

(makefile
  (function_call
    (arguments
      (argument
        (word))
      (argument
        (word))
      (argument
        (word))
      (argument
        (word)))))

================================================================================
foreach function with embedded calls
================================================================================
$(foreach prog,$(PROGRAMS),$(eval $(call PROGRAM_template,$(prog))))

--------------------------------------------------------------------------------

(makefile
  (function_call
    (arguments
      (argument
        (word))
      (argument
        (variable_reference
          (word)))
      (argument
        (function_call
          (arguments
            (argument
              (function_call
                (arguments
                  (argument
                    (word))
                  (argument
                    (variable_reference
                      (word))))))))))))

================================================================================
call error function with 1 argument
================================================================================
$(error e e)

--------------------------------------------------------------------------------

(makefile
  (function_call
    (arguments
      (argument
        (word)))))

================================================================================
call to error function with 2 arguments
================================================================================
$(error e e,e e)

--------------------------------------------------------------------------------

(makefile
  (function_call
      (arguments
        (argument
          (word))
        (argument
          (word)))))

================================================================================
explicit call to shell function with 2 arguments
================================================================================
$(shell echo hello)

--------------------------------------------------------------------------------

(makefile
  (shell_function
    (shell_command)))

================================================================================
implicit call to shell function with 2 arguments
================================================================================
$$(echo hello)

--------------------------------------------------------------------------------

(makefile
  (shell_expression
    (shell_command)))
