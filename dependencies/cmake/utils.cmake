# utility functions

# get remote repository url 
function(get_git_remote_origin_url output_var working_dir)
  execute_process(
    COMMAND git config --get remote.origin.url
    WORKING_DIRECTORY ${working_dir}
    OUTPUT_VARIABLE git_url
    ERROR_QUIET # Suppress error output if no git repo
    RESULT_VARIABLE git_result
  )

  if(git_result EQUAL 0)
    string(STRIP "${git_url}" git_url) # Remove leading/trailing whitespace
    set(${output_var} "${git_url}" PARENT_SCOPE)
  else()
    # Handle the case where there's no git repo or no remote origin.
    # Set a default value, an empty string, or handle the error as needed.
    message(STATUS "No git repository or remote origin found.")
    set(${output_var} "" PARENT_SCOPE) # Or set a default URL
    # Or you could:
    # message(FATAL_ERROR "No git repository or remote origin found.")
  endif()
endfunction()

# check if a Git repository URL is valid
function(check_git_repo url result)
    execute_process(
        COMMAND git ls-remote ${url}
        RESULT_VARIABLE res
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(${res} EQUAL 0)
        set(${result} TRUE PARENT_SCOPE)
    else()
        set(${result} FALSE PARENT_SCOPE)
    endif()
endfunction()