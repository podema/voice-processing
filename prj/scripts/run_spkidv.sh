#!/bin/bash

# Scripting is very useful to repeat tasks, as testing different configuration, multiple files, etc.
# This bash script is provided as one example
# Please, adapt at your convinience
# Antonio Bonafonte, April 2013



# Set the proper value to the next variables

w=$HOME/tmp # work directory 
db=$HOME/Descargas/speecon # directory with the input database
pavbin=$HOME/bin/release # directory with the programs

# Add the path of bin files to the path where the operative system looks for 'programs'
PATH=$PATH:$pavbin

CMDS="lists mcp d1c d2c gmm_mcp test_mcp finaltest"

if [[ $# < 1 ]]; then
   echo "$0 cmd1 [...]"
   echo "Where commands can be:"
   echo "    lists: create, for each spk, training and devel. list of files"
   echo "      mcp: feature extraction (mel cepstrum parameters)"
#   echo "      d1c: 1st derivative"
#   echo "      d2c: 2nd derivative"
   echo "   gmm_mcp: train gmm for the mcp features"
   echo " background: compute the background model"   
   echo " verify: run a verify test with candidates.list"
   echo "  test_mcp: test GMM using only mcp features"
   exit 1
fi


for cmd in $*; do
   echo `date`: $cmd '---';

   if [[ $cmd == lists ]]; then
       \rm -fR $w/lists
       mkdir -p $w/lists
       for dir in $db/BLOCK*/SES* ; do

	   name=${dir/*\/}
	   echo Create list for speaker $dir $name ----
	   (find $db/BLOCK*/$name -name "*.wav" | perl -pe 's/^.*BLOCK/BLOCK/; s/\.wav$//' | unsort > $name.list) || exit 1
	   # split in test list (5 files) and train list (other files)
	   (head -5 $name.list | sort > $w/lists/$name.test) || exit 1
	   (tail -n +6 $name.list | sort > $w/lists/$name.train) || exit 1
	   \rm -f $name.list
       done
       cat $w/lists/*.train | sort > $w/lists/all.train
       cat $w/lists/*.test | sort > $w/lists/all.test
   elif [[ $cmd == mcp ]]; then
       for line in $(cat $w/lists/verif_files.txt); do
	   mkdir -p `dirname $w/mcp/$line.mcp`
	   echo "$db/$line.wav" "$w/mcp/$line.mcp"
	   wav2mcp "$db/$line.wav" "$w/mcp/$line.mcp" || exit 1
       done
   elif [[ $cmd == gmm_mcp ]]; then
       for dir in $db/BLOCK*/SES* ; do
	   name=${dir/*\/}
	   echo $name ----
	   gmm_train -v 1 -m 12 -d $w/mcp -e mcp -g $w/gmm/mcp/$name.gmm $w/lists/$name.train 
           echo
       done
  elif [[ $cmd == test_mcp ]]; then
       find $w/gmm/mcp -name '*.gmm' -printf '%P\n' | perl -pe 's/.gmm$//' | sort  > $w/lists/gmm.list
       gmm_classify -d $w/mcp -e mcp -D $w/gmm/mcp -E gmm $w/lists/gmm.list  $w/lists/all.test | tee $w/result.log
       perl -ne 'BEGIN {$ok=0; $err=0}
                 next unless /^.*SA(...).*SES(...).*$/; 
                 if ($1 == $2) {$ok++}
                 else {$err++}
                 END {printf "nerr=%d\tntot=%d\terror_rate=%.2f%%\n", ($err, $ok+$err, 100*$err/($ok+$err))}' $w/result.log

  elif [[ $cmd == background ]]; then
    gmm_train -v 1 -m 12 -d $w/mcp -e mcp -g $w/gmm/mcp/background.gmm $w/lists/all.train

elif [[ $cmd == verify ]]; then
    gmm_verify -d $w/mcp -e mcp -D $w/gmm/mcp -E gmm $w/lists/gmm.list  $w/verif_files.txt $w/verif_target.txt | tee $w/resultv.log

   elif [[ $cmd == final_test ]]; then
       echo "To be implemented ..."
   else
       echo "undefined command $cmd" && exit 1
   fi
done

exit 0