#!/usr/bin/perl

#
# $ Copyright Cypress Semiconductor $
#

if (! $ARGV[0] )
{
    print "Usage ./bin_to_c.pl  <variable name> <bin file>";
    exit;
}

# Print start of output
$variable_name = shift @ARGV;
$original_variable_name = $variable_name;
$file = shift @ARGV;

#open the file
open INFILE, "<:raw", $file or die "cant open " . $file;
@file_cont_array = <INFILE>;
close INFILE;
$file_cont = join('',@file_cont_array);


print "const unsigned char ${variable_name}[" . (length( $file_cont )) . "] = {\n";
my @vals = unpack( "C*", $file_cont );

my $linepos;
my $firstval = 1;

foreach $val (@vals)
{
    my $valsize = 1;
    if ( ( $val >= 10 ) && ( $val < 100 ) )
    {
        $valsize = 2;
    }
    elsif ( $val >= 100 )
    {
        $valsize = 3;
    }

    if ( $firstval == 1 )
    {
        print "        $val";
        $linepos = 8 + $valsize;
        $firstval = 0;
    }
    elsif ( $linepos + 5 >= 79 )
    {
        print ",\n        $val";
        $linepos = 8 + $valsize;
    }
    else
    {
        print ", $val";
        $linepos += 2 + $valsize;
    }
}

print "\n};\n";


