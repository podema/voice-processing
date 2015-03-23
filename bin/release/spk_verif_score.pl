#!/usr/bin/perl -w

$show_threshold_search = 1;
$p_target = 0.01;
$p_fa = 1 - $p_target;
$N_thr = 100; #number of thresholds to test


if ($p_target < 0.5) {
    $cost_norm = $p_target;
} else {
    $cost_norm =  1-$p_target;
}

if (@ARGV != 1 && @ARGV != 2) {
    print STDERR "Usage: $0 threshold file1.results\n";
    print STDERR "       $0 file1.results\n\n";
    print STDERR "In the second call, the optimum threshold is found\n";
    exit 1;
}


sub count_gt {
    $thr = shift;
    $n = 0;
    foreach $item (@_) {
	$n++ if $item > $thr;
    }
    return $n;
}

sub read_data {
    while (<>) {
	next if /^\s*$/;  # skip white lines
	chomp;

	if (3 > (($filename, $spkid, $score) = split)) {
	    print STDERR "Format error in line $_\n";
	    exit 1;	
	}
	
	$user = "";
	if ($filename !~ /.*SA(...)/) {
	    print STDERR "Format error in fname ($filename), line $_\n";
	    exit 1;
	}
	$user = $1;
	if ($spkid !~ /SES(...)/) {
	    print STDERR "Format error in spk ($spkid), line $_\n";
	    exit 1;
	}
	$spkid = $1;
	
	$min_score = $score if $min_score > $score;
	$max_score = $score if $max_score < $score;
	
	push @users, $score if ($spkid eq $user);
	push @impostors, $score if ($spkid ne $user);	
    }
}


# Process
@users = ();
@impostors = ();
$min_score = 1e30;
$max_score = -1e30;
$threshold = -1e30;
$threshold = shift if (@ARGV == 2);
read_data;

$n_users = int(@users);
$n_impostors = int(@impostors);
#print "$n_users\t$n_impostors\n";
if ($n_users == 0 || $n_impostors == 0) {
    print STDERR "Cannot compute cost. n_users=$n_users; n_impostor=$n_impostors\n";
    exit 1;
}


if ($threshold == -1e30) {
    $thr_step = ($max_score-$min_score)/$N_thr;
    $min_cost = 1e30;
    print "Threshold search:\n"
	if $show_threshold_search;

    for ($thr = $min_score; $thr < $max_score+$thr_step; $thr += $thr_step) {
	$n_falseAlarm = count_gt($thr, @impostors);
	$n_miss = $n_users - count_gt($thr, @users);
	$p_miss = $n_miss/$n_users;
	$p_falseAlarm = $n_falseAlarm/$n_impostors;
	$cost = $p_miss * $p_target + $p_falseAlarm * (1-$p_target);
	$cost = $cost/$cost_norm;
	if ($cost < $min_cost) {
	    $min_cost = $cost;
	    $threshold = $thr;
	}
	print "THR\t$thr\t$cost\t$n_miss\t$n_falseAlarm\n" 	
	    if $show_threshold_search;

    }
    print "\n\n" if $show_threshold_search;
} 

$n_falseAlarm = count_gt($threshold, @impostors);
$n_miss = $n_users -count_gt($threshold, @users);
$p_miss = $n_miss/$n_users;
$p_falseAlarm = $n_falseAlarm/$n_impostors;
$cost = $p_miss * $p_target + $p_falseAlarm * (1-$p_target);
$cost = $cost/$cost_norm;


print "==============================================\n";
print "THR: $threshold\n";
print "Missed:     $n_miss/$n_users=", sprintf("%.4f\n",$p_miss);
print "FalseAlarm: $n_falseAlarm/$n_impostors=", sprintf("%.4f\n", $p_falseAlarm);
print "----------------------------------------------\n";
print "==> CostDetection: ", sprintf("%d\n", 100*$cost); 
print "==============================================\n";

exit 0;
