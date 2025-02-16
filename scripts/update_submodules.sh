#!/bin/bash
# -----------------------------------------------------------------------------
# Script:       update_submodules.sh
# Author:       Guillaume Plante
# Description:  Displays all users currently logged in, the login time of
#               their session, and the session duration (time elapsed since login).
#
#         1. Set Submodule Branch:  
#           The script runs `git submodule foreach 'git config submodule.$name.branch main'` 
#           to ensure that each submodule's configuration tracks the "main" branch. This prevents future 
#           `git submodule update --remote` calls from leaving the submodule in a detached HEAD.
#    
#         2. Check and Switch Branch:  
#           For each submodule, it checks the current branch using `git symbolic-ref`. 
#           If the submodule is in a detached HEAD state or on a branch other than "main", it will check out the "main" branch.
#    
#         3. Pull Latest Changes:  
#           Once on the main branch, it pulls the latest changes from the remote "main" branch.
#    
#         4. Update Main Repository:  
#           Finally, the script adds and commits any updated submodule references in 
#           the main repository. (If no changes exist, it simply outputs that there are no changes.)
#
# -----------------------------------------------------------------------------

# This script updates all repository submodules:
# 1. It checks out the "main" branch in each submodule (if not already).
# 2. It pulls the latest changes from the remote "main" branch.
# 3. It avoids leaving the submodules in a detached HEAD state.

# Ensure you are in the repository root.
echo "Updating all submodules to track the main branch..."

# Update the .gitmodules file (if needed) so that each submodule is configured
# to track the main branch.
git submodule foreach 'git config submodule.$name.branch main'

# Iterate through each submodule.
git submodule foreach '
  # Determine current branch (if in detached HEAD, branch will be empty).
  current_branch=$(git symbolic-ref --short -q HEAD)
  if [ -z "$current_branch" ]; then
      echo "Submodule $name is in detached HEAD; checking out main..."
      git checkout main || { echo "Failed to checkout main in $name"; exit 1; }
  elif [ "$current_branch" != "main" ]; then
      echo "Submodule $name is on branch \"$current_branch\"; switching to main..."
      git checkout main || { echo "Failed to switch to main in $name"; exit 1; }
  else
      echo "Submodule $name is already on main."
  fi
  echo "Pulling latest changes in submodule $name..."
  git pull origin main || { echo "Failed to pull in $name"; exit 1; }
'

# Optionally, update the submodule references in the main repository.
echo "Updating main repository submodule references..."
git add -u
git commit -m "Updated submodules to latest main branch commits" || echo "No changes to commit."
echo "Submodule update complete."
