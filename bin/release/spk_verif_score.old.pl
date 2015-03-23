#!/usr/bin/perl -w

if (@ARGV < 1) {
    print STDERR "Usage: $0 threshold [file1.log ...]\n";
    exit 1;
}

$threshold = shift;

$n_user = 0; $n_miss = 0;
$n_impostor=0; $n_falseAlarm = 0;
while (<>) {
    next if /^\s*$/;
    chomp;
    if (3 != (($filename, $spkid, $score) = split)) {
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

    if ($spkid eq $user) {
	$n_user++;
	$n_miss++ if ($score < $threshold);
    }

    if ($spkid ne $user) {
	$n_impostor++;
	$n_falseAlarm++ if ($score > $threshold);
    }

}

if ($n_user == 0 || $n_impostor == 0) {
    print STDERR "Cannot compute cost. n_user=$n_user; n_impostor=$n_impostor\n";
    exit 1;
}

$p_miss = $n_miss/$n_user;
$p_falseAlarm = $n_falseAlarm/$n_impostor;

$cost = $p_miss * 0.001 + $p_falseAlarm * (1-0.001);

print "==============================================\n";
print "Missed:     $n_miss/$n_user=$p_miss\n";
print "FalseAlarm: $n_falseAlarm/$n_impostor=$p_falseAlarm\n";
print "----------------------------------------------\n";
print "==> CostDetection: ", sprintf("%.2f", 100*$cost); 
print "%\n";
print "==============================================\n";

exit 0;
