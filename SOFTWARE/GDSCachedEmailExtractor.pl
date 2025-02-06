#!/usr/bin/perl

################################################################################
################################################################################
#
# Download all cached emails from Google Desktop Search [Google Desktop 5.1.0705.04505-en-pb]
#
#	Released under GPL.
#
#		by Andrew Hardwick, http://duramecho.com
#
#################################################################################
################################################################################
# Version 1, 2009/9/11
# Version 2, 2009/9/12
#  Started page navigation from GDS home page instead of email timeline.
################################################################################
# Instructions:
#  Open GDS, copy the URL of the GDS home page then run this program from
#   the command line with that URL as the command line parameter. I.e.type:
#   perl GDSCachedEmailExtractor.pl "<GDS home page URL>" <Enter>
#  This was written for Google Desktop version 20051208-en and it heavily
#   depends on the pages returned by GDS to of the format that version
#   has been found to return. If you want to use it on another version,
#   try it and you may be lucky in that the pages returned are sufficiently
#   similar. If they have small changes in formatting of the HTML then
#   it may be possible to adapt this program by change it the regular
#   expressions in the settings (the 'Regexp...' settings) to match. If
#   the site structure returned has changed then this program will need
#   modification.
#  It should only take the cached emails that are not still in the email
#   client's mailbox (GDS provides different format links to those so it can
#   open the originals in the email client instead of the cached copy so this
#   program ignores them).
#  Note that this source code is very verbose. That is to make it
#   easier to understand as sample code.
###############################################################################
##### This file is formatted for 80 character lines and 4 character tabs. #####
###############################################################################

# Settings
my $RegexpTimeline='<font size=-1><a href=\'(.+?)\'>Browse Timeline<\/a>';
my $RegexpTimelineEmail='<a href=\'(.+?)\'>emails<\/a>';
my $RegexpPreviousDate='<font size=-1><a href=\'(.+?)\'>&lt; Older<\/a>';
my $RegexpCachedEmailRedirector='<a href=\'(\/redir\?url='.
		'http%3A%2F%2F127%2E0%2E0%2E1%3A4664%2Fcache%3Fevent.+?)\'>';
my $RegexpCachedEmail='<meta http-equiv="refresh".*?url=\'(.+?)\'">';

###############################################################################
# Load libraries
use strict;			# Ban automatic variables
use LWP::Simple;	# For 'get'

###############################################################################

{	# Get URL of the GDS home page as the command line argument
	my $GDSStartPage=$ARGV[0];
	unless($GDSStartPage)
	{	die "Error: Google Destop Search home page URL should be provided ".
				"as the command line parameter.";}
	# Get URL of lastest timeline page
	my $PageStart=get($GDSStartPage);
	$PageStart=~/$RegexpTimeline/i;
	my $UrlTimeline="http://127.0.0.1:4664$1";
	print $UrlTimeline."\n";
	# Get URL of lastest timeline page restricted to emails
	my $PageTimeline=get($UrlTimeline);
	$PageTimeline=~/$RegexpTimelineEmail/;
	my $UrlTimelineEmail="http://127.0.0.1:4664$1";
	print $UrlTimelineEmail."\n";
	# Create output directory
	mkdir('RecoveredEmail',0777);
	chdir('RecoveredEmail');
	# Start numbering for recovered emails
	my $EmailNumber=0;
	# Iterate over Timeline pages (starting with the most recent)
	while($UrlTimelineEmail)
	{	# Get the Timeline page
		my $PageTimelineEmail=get($UrlTimelineEmail);
		# Iterate over emails linked to from that Timeline page
		while($PageTimelineEmail=~/$RegexpCachedEmailRedirector/g)
		{	# Get the page that redirects to the cached email page
			my $UrlCachedEmailRedirector="http://127.0.0.1:4664$1";
			my $PageCachedEmailRedirector=get($UrlCachedEmailRedirector);
			# Get the cached email page
			$PageCachedEmailRedirector=~/$RegexpCachedEmail/;
			my $UrlCachedEmail=$1;
			my $PageCachedEmail=get($UrlCachedEmail);
			# Save the email page to a file
			open(File,">$EmailNumber.html");
			print File $PageCachedEmail;
			close File;
			print "Emails retrieved so far = ".++$EmailNumber."\n";}
		# Find previous Timeline page
	
		if($PageTimelineEmail=~/$RegexpPreviousDate/)
		{	
			$UrlTimelineEmail="http://127.0.0.1:4664$1";
			print "New Link: ".$UrlTimelineEmail."\n";
		}
		else
		{	print "Finished!\n";
			print $PageTimelineEmail."\n_________________________________________\n";
			$PageTimelineEmail=~/font size=\"-1\"><a href=\"(.+?)\">&lt; Older<\/a>'/;
			$UrlTimelineEmail="http://127.0.0.1:4664$1";
			print "New LinkElse: ".$UrlTimelineEmail."\n";
			$UrlTimelineEmail=0;
		}
	}
}

###############################################################################