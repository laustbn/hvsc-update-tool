#!/usr/bin/perl
#
# Orchestrate testing the update executable on a Windows 10 machine. Upload exe
# and test script, run test, verify successful.
#
# SSH logs into PowerShell. We can't reliably use exit codes, so this wrapper
# explicitly checks the output of the script.
#
# TODO: this is very quick and dirty. Long term this should be rewritten in
# Python, but this was faster.

use v5.40;
use warnings;
use Data::Dumper;
use IPC::Run3;

my $user = "user";
my $host = "hvsc-win10";
my $target = "z:/test";

my $exe = shift @ARGV or die "Missing exe argument";

chomp(my $root = `git rev-parse --show-toplevel`);
chdir($root);

chomp(my @test_files = `git ls-files test/`);

sub info {
    say localtime . "|", @_;
}

sub ssh_bash_pipe {
    # Wrapper for invoking bash and piping commands via STDIN. That avoids some of
    # the quoting/escaping headache otherwise present in this setup.
    my @cmd = @_;
    my $in = join("\n", @cmd);
    my $out;
    my $err;
    my @ssh = ("ssh", "-oBatchMode=yes", "$user\@$host", "sh");

    run3 \@ssh, \$in, \$out, \$err;
    return $out, $err;
}

sub scp {
    my $dst = pop @_;
    my @source = @_;
    my $res = system("scp", @source, "$user\@$host:$dst");
    die "scp of @source to $dst failed with $?" unless $res == 0;
}

sub check_ssh_works {
    my @res = ssh_bash_pipe("echo It_Works");
    my $success = grep /^It_Works/, @res;
    return $success == 1;
}

check_ssh_works || die "SSH failing";

ssh_bash_pipe("mkdir -p /z:/hvsc/test");
scp (@test_files, "/z:/hvsc/test/");
scp ($exe, "/z:/hvsc/test/update.exe");

# Run test
info "Running test, this will take a while";
my ($out, $err) = ssh_bash_pipe("cd z:/hvsc/test; ./test.py --exe ./update.exe test");

my $success = $out =~ /All updates verified/;

if ($success) {
    info "Test passed";
} else {
    info "Test failed";
}

# Save output for inspection
{
    open (my $fh, ">", "test.stdout");
    print $fh $out;
    close $fh;
}

{
    open (my $fh, ">", "test.stderr");
    print $fh $err;
    close $fh;
}
