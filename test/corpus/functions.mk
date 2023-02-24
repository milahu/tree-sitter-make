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
call to error function
================================================================================
$(error)

--------------------------------------------------------------------------------

(makefile
  (function_call
    (arguments
      (argument
        (MISSING word)))))

================================================================================
call to error function with 1 argument
================================================================================
$(error e)

--------------------------------------------------------------------------------

(makefile
  (function_call
    (arguments
      (argument
        (word)))))

================================================================================
call to error function with 2 arguments
================================================================================
$(error e e2)

--------------------------------------------------------------------------------

(makefile
  (function_call
    (ERROR
      (arguments
        (argument
          (word))))
    (arguments
      (argument
        (word)))))
