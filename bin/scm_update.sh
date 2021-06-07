#!/bin/bash -l


readonly SCRIPT_DIR=$( cd "$( dirname $(realpath "${BASH_SOURCE[0]}") )" >/dev/null && pwd -P )
source "${SCRIPT_DIR}/_init.sh"


repo=$(git remote -v | grep '^origin\b.*\bpush)$' | awk '{ print $2 }')

#=== 'prev-commit' solution by o_O Tync
#commit_hash=$(git rev-parse --verify HEAD)
commit=$(git log -1 --pretty="%H%n%cI") # hash \n date
commit_hash=$(echo "$commit" | head -1)
commit_date=$(echo "$commit" | head -2 | tail -1) # 2010-12-28 05:16:23 +0300

branch_name=$(git symbolic-ref -q HEAD) # http://stackoverflow.com/questions/1593051/#1593487
branch_name=${branch_name##refs/heads/}
branch_name=${branch_name:-HEAD} # 'HEAD' indicates detached HEAD situation


# sketch
cat <<EOT > "$SKETCH_DIR/scm-generated.h"
#ifndef SCM_GENERATED_H_
#define SCM_GENERATED_H_

#define SCM_REPO "$repo"
#define SCM_HASH "$commit_hash"
#define SCM_DATE "$commit_date"
#define SCM_CHAN "$branch_name"

#endif // SCM_GENERATED_H_
EOT


exit 0
