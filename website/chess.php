#!/usr/bin/php
<?php
	/*
		chess.php
		This is the main script file used to control the chess server
	*/
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
	header( "Content-Type: text/xml" );

	// get form variables:
	$newgame = $_POST['inputnewgame'];
	$from = $_POST['inputsquarefrom'];
	$to = $_POST['inputsquareto'];
	$pieceplacement = $_POST['inputpieceplacement'];
	$castleavailability = $_POST['inputcastleavailability'];
	$enpassant = $_POST['inputenpassant'];
	$activecolor = $_POST['inputactivecolor'];
	$halfmoveclock = $_POST['inputhalfmoveclock'];
	$fullmovenumber = $_POST['inputfullmovenumber'];

	$execstring = "";
	
	if ($newgame == "1") {
		$execstring = " -n";
	}
	else
	{
		if ($from != "")
			$move = $from.":".$to;
			
		$execstring = " -p".$pieceplacement." -c".$castleavailability." -e".$enpassant." -a".$activecolor." -l".$halfmoveclock." -f".$fullmovenumber." -m".$move." -v2";
	}
	
	// call chess server
	$myoutput = passthru('cgi-bin/chess -x'.$execstring);

	// return results
	print $myoutput;

?>
