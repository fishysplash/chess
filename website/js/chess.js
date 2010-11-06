// -------------------------------------------------
// Board object
// -------------------------------------------------
var board = new Object;
board.top = 0;	// default
board.left = 0;	// default
board.squareSize = 40;	// default
board.pieces = new Array();	// pieces

board.getPieceCount = function()
{
	return board.pieces.length;
}

// adds a piece to board
board.addPiece = function(sPiece, sSAN)
{
	var square = new Square(sSAN);
	var newId = Math.floor(Math.random()*10000000);		// assign random id, since pieces added/deleted therefore cannot use length.
	var id = "piece_" + newId;
	board.pieces[board.getPieceCount()] = new Piece(id,square.row,square.col,sPiece);

	return id;
}

// remove piece from board and redimension array
board.removePiece = function(oPiece)
{
	// remove image
	var oImg = oPiece.targetImage;
	document.body.removeChild(oImg);

	var isFound=false;
	for (var i=0; i<board.pieces.length; i++)
	{
		if (isFound)
		{
			board.pieces[i-1] = board.pieces[i];
		}

		else if(board.pieces[i].id == oPiece.id)
		{
			isFound=true;
			board.pieces[i]=undefined;
		}
	}
	board.pieces.length=board.pieces.length-1;
}

// returns a piece object based on id.
board.getPieceById = function(sPieceId)
{
	for (var i=0; i<board.pieces.length; i++)
	{
		if (board.pieces[i].id==sPieceId)
		{
			return board.pieces[i];
		}
	}
}

board.removeAllPieces = function()
{
	while(board.pieces.length>0)
	{
		board.removePiece(board.pieces[0]);
	}
}

board.drawwaitgif = function(color)
{
	var oImg = document.createElement("img");
	document.body.appendChild(oImg);

	oImg.setAttribute("id","img" + color + "wait");
	oImg.setAttribute("src","./images/" + color + "_wait.gif");

	oImg.style.position="absolute";
	var leftText = (board.left - (oImg.width/2) +(board.squareSize*4)).toString() + "px";
	var topText = (board.top - (oImg.width/2) + (board.squareSize*(4))).toString() + "px";
	oImg.style.left = leftText;
	oImg.style.top = topText;

	oImg.style.opacity=0.0;	// transparent initially
	oImg.style.filter = "alpha(opacity = 0)";	//IE
}

board.showwaitgif = function(color)
{
	board.hidewaitgif();
	var oImg = document.getElementById("img" + (color.substring(0,1).toLowerCase()=="b" ? "black" : "white") + "wait");

	oImg.style.left = (board.left - (oImg.width/2) +(board.squareSize*4)).toString() + "px";
	oImg.style.top = (board.top - (oImg.width/2) + (board.squareSize*(4))).toString() + "px";
	oImg.style.opacity = 1.0;
	oImg.style.filter = "alpha(opacity = 100)";
}

board.hidewaitgif = function()
{
	document.getElementById("imgwhitewait").style.opacity = 0.0;
	document.getElementById("imgwhitewait").style.filter = "alpha(opacity = 0)";
	document.getElementById("imgblackwait").style.opacity = 0.0;
	document.getElementById("imgblackwait").style.filter = "alpha(opacity = 0)";
}

// call when the drag/drop has finished
board.finishmove = function()
{
	board.showwaitgif("black");
	
	// player completed their move - process it (ie check if valid
	// and if so, let next player play
	serverGateway.send();		// do server move.

	// put the piece back to where it started, so computer can 'play' the full move
	// if the move is illegal, the piece is back where it started as well.
	startSquare = new Square(document.getElementById("inputsquarefrom").value);
	savedTarget.style.top = startSquare.top + "px";
	savedTarget.style.left = startSquare.left + "px";
}

// draws the board
board.draw = function()
{
	var col;

	// board frame
	document.getElementById("divBoard").innerHTML = document.getElementById("divBoard").innerHTML +	
		"<table id='boardframe' style='" + 
		"top: " + (board.top) + "px; " + 
		"left: " + (board.left) + "px; " +
		"width: " + (board.squareSize*8) + "px; " +
		"height: " + (board.squareSize*8) + "px;'><tr><td></td></tr></table>";

	// wait gifs
	board.drawwaitgif("black");
	board.drawwaitgif("white");
	
	// squares	
	for (var square=0; square < 64; square++)
	{
		col = square % 8;
		row = Math.floor(square / 8);
		var squareClass = ((row+col) % 2 == 0) ? "whiteSquare" : "blackSquare";	
		var leftText = (board.left + (board.squareSize*col)).toString();
		var topText = (board.top + (board.squareSize*row)).toString();
		var widthText = board.squareSize.toString();
		var heightText = board.squareSize.toString();
		document.getElementById("divBoard").innerHTML = document.getElementById("divBoard").innerHTML +
			"<div class='" + squareClass + "' id='" + square + "' style='position: absolute; " +
			"left: " + leftText + "px; " +
			"top: " + topText + "px; " +
			"width: " + widthText + "px; " +
			"height: " + heightText + "px;' />";
	}
}

// Gets the square according to a 'moving' target. This is preferred way of updating
// the From/To squares. Can be different to getSquare() depending on whether user
// has clicked the center or edge of a piece.
board.getSquareForTarget = function(oTarget)
{
	// Gets the square that mouse event is 'on' (0-based)
	var col = Math.round((parseInt(oTarget.style.left) - board.left) / board.squareSize);
	var row = 7 - Math.round((parseInt(oTarget.style.top) - board.top) / board.squareSize);
	return new Square(row,col);
}

// Gets the square according to the mouse pointer.
board.getSquare = function(e)
{
	// Gets the square that mouse event is 'on' (0-based)
	var col = Math.round((parseInt(e.clientX) + (board.squareSize/2) - board.left) / board.squareSize) -1 ;
	var row = 8 - Math.round((parseInt(e.clientY) + (board.squareSize/2) - board.top) / board.squareSize);
	return new Square(row,col);
}

board.snapToSquare = function(target)
{
	// when moving a piece, snap to square
	var col = Math.floor((parseInt(target.style.left) + (board.squareSize/2) - board.left) / board.squareSize);
	target.style.left = board.left + (col*board.squareSize) + "px";

	var row = 8 - Math.floor((parseInt(target.style.top) + (board.squareSize/2) - board.top) / board.squareSize);
	target.style.top = board.top + ((8-row)*board.squareSize) + "px";
}

// -------------------------------------------------
// class to represent a single chess square.
// -------------------------------------------------
function Square(iRow, iCol)
{
	var colString = "abcdefgh";

	if (arguments==null)
		return;

	// alternative way of  instantiating a square is passing in the SAN
	if (arguments.length==1)
	// iRow contains SAN text (eg 'e4')
	{
		var col = iRow.substring(0,1);
		this.col = colString.indexOf(col);
		this.row = parseInt(iRow.substring(1))-1;
	}					
	else	// passed in row number and col number
	{
		this.row = iRow;
		this.col = iCol;
	}

	this.top = ((7-this.row) * board.squareSize) + board.top;
	this.left = (this.col * board.squareSize) + board.left;
	this.number = (this.row*8)+this.col;

	if (typeof Square._initialized == "undefined")
	{
		Square.prototype.toString = function()
		{
			var cols = "abcdefgh";
			return cols.substring(this.col,this.col+1) + (this.row+1);
		};
	}
	Square._initialized = true;
}

// -------------------------------------------------
// class to represent a single chess piece.
// -------------------------------------------------
function Piece(sId, iRow, iCol, sPiece)
{
	this.id = sId;
	this.color = (sPiece.toUpperCase()==sPiece) ? "white" : "black";	// 'white' or 'black'

	switch(sPiece.toUpperCase())
	{
		case "P":
			this.piece="pawn";
			break;
		case "R":
			this.piece="rook";
			break;
		case "N":
			this.piece="knight";
			break;
		case "B":
			this.piece="bishop";
			break;
		case "Q":
			this.piece="queen";
			break;
		case "K":
			this.piece="king";
			break;
		default:
			this.piece=sPiece;
	}

	// add visual piece onto board.
	var oImg = document.createElement("img");
	document.body.appendChild(oImg);

	oImg.style.position="absolute";
	var leftText = (board.left + (board.squareSize*iCol)).toString() + "px";
	var topText = (board.top + (board.squareSize*(7-iRow))).toString() + "px";
	var widthText = board.squareSize.toString() + "px";
	var heightText = board.squareSize.toString() + "px";

	oImg.style.left = leftText;
	oImg.style.top = topText;
	oImg.style.width = widthText;
	oImg.style.height = heightText;
	oImg.setAttribute("id",this.id);
	oImg.setAttribute("src","./images/" + this.color + "_" + this.piece + ".gif");
	oImg.style.opacity=0.0;	// transparent initially
	oImg.style.filter = "alpha(opacity = 0)";	//IE

	this.targetImage = oImg;

	if (typeof Piece._initialized == "undefined")
	{
		Piece.prototype.square = function()
		{
			var row = 7 - ((parseInt(this.targetImage.style.top) - board.top) / board.squareSize);
			var col = ((parseInt(this.targetImage.style.left) - board.left) / board.squareSize);
			return new Square(row,col);
		};

	}
	Piece._initialized = true;
	oImg.setAttribute("alt",this.id);	// the piece id
}


var pieceToMoveStartTime;	// Used to get start time of move
var duration = 1500;		// 1.5 second duration for moves
var frameLength=50;		// 50ms framelength
var pieceToMoveIntervalId;	// to store js timer
var pieceToMove = new Array();
var pieceToMoveStartSquare = new Array();
var pieceToMoveFinishSquare = new Array();
var pieceAdding = new Array();	// if adding piece, this is set to true
var pieceDeleting = new Array(); // if deleting piece, this is set to true

// central function which processes each turn.
function processTurn()
{
	if (document.getElementById("inputnewgame").value == "1")
	{
		serverGateway.send();		// process for new game
		document.getElementById("inputnewgame").value == "0"
	}
	else if (document.getElementById("inputblackauto").checked==true && document.getElementById("inputactivecolor").value=="b")
	{
		serverGateway.send();		// process black if computer
	}
	else if (document.getElementById("inputwhiteauto").checked==true && document.getElementById("inputactivecolor").value=="w")
	{
		serverGateway.send();		// process black if computer
	}
	else
	{
		// do nothing - let human make a move first...
	}
}

function setUIMoveText(sMovetext)
{
	// moves pieces on screen according to movetext
	// can deal with multiple moves/deletes/adds
	var moves = sMovetext.split(" ");
	var rePieces = /\b(p|P|r|R|n|N|b|B|q|Q|k|K)\b/;
	var iIndex=-1;
	for (var iMove=0; iMove<moves.length; iMove++)
	{
		iIndex++;
		var singleMoveComponents = moves[iMove].split(":");
		var moveStart = singleMoveComponents[0];
		var moveFinish = singleMoveComponents[1];

		if (moveStart == "CLEAR" && moveFinish == "PIECES")
		{
			board.removeAllPieces();
			iIndex--;
		}

		// adding a piece ???
		else if (rePieces.test(moveStart)==true)
		{
			var newPiece = moveStart;
			var newID = board.addPiece(newPiece, new Square(moveFinish).toString());
			pieceToMove[iIndex] = board.getPieceById(newID);	// document.getElementById(newID);
			pieceToMoveStartSquare[iIndex] = new Square(moveFinish);
			pieceToMoveFinishSquare[iIndex] = pieceToMoveStartSquare[iIndex];
			pieceAdding[iIndex] = true;
		}

		else
		{
			// moving a piece
			for (var i=0; i < board.pieces.length; i++)
			{
				if (board.pieces[i].square().toString() == moveStart)
				{
					pieceToMove[iIndex] = board.getPieceById(board.pieces[i].id);	
					pieceToMoveStartSquare[iIndex] = board.getSquareForTarget(pieceToMove[iIndex].targetImage);
					break;
				}
			}

			// deleting a piece ???
			if (rePieces.test(moveFinish)==true)
			{
				pieceDeleting[iIndex] = true;
			}
			else
			{
				pieceToMoveFinishSquare[iIndex] = new Square(moveFinish);
			}
		}
	}

	// Set Javascript interval
	var d = new Date();	
	pieceToMoveStartTime = d.getTime();
	pieceToMoveIntervalId = window.setInterval("animatePieces()", frameLength);
}

function animatePieces()
{
	// this function moves potentially multiple pieces (eg for castling).
	var currTime = new Date().getTime();
	var prctComplete = (currTime - pieceToMoveStartTime) / duration;

	for (var i=0; i<pieceToMove.length; i++)
	{
		if(pieceAdding[i]==true)
		{
			// new piece to 'appear'
			pieceToMove[i].targetImage.style.opacity = prctComplete;
			pieceToMove[i].targetImage.style.filter = "alpha(opacity = " + (prctComplete*100) + ")";	//IE
		}

		else if(pieceDeleting[i]==true)
		{
			// piece to 'disappear'
			pieceToMove[i].targetImage.style.opacity = 1 - prctComplete;
			pieceToMove[i].targetImage.style.filter = "alpha(opacity = " + (100 - (prctComplete*100)) + ")";	//IE

		}

		else	// just move the piece
		{
			pieceToMove[i].targetImage.style.top = (pieceToMoveStartSquare[i].top + ((pieceToMoveFinishSquare[i].top -
	 			pieceToMoveStartSquare[i].top) * prctComplete)) + "px";
	 		pieceToMove[i].targetImage.style.left = (pieceToMoveStartSquare[i].left + ((pieceToMoveFinishSquare[i].left -
	 			pieceToMoveStartSquare[i].left) * prctComplete)) + "px";
		}
 		if (currTime - pieceToMoveStartTime > duration)
		{
			window.clearInterval(pieceToMoveIntervalId);
			// At end of animation, ensure piece snaps to correct square
			// even if animation is bit wonkey.
			if (pieceToMoveFinishSquare[i]!=undefined)
			{
				pieceToMove[i].targetImage.style.top = pieceToMoveFinishSquare[i].top + "px";
				pieceToMove[i].targetImage.style.left = pieceToMoveFinishSquare[i].left + "px";
			}
			board.snapToSquare(pieceToMove[i].targetImage);
			
			if(pieceDeleting[i]==true)
			{
				board.removePiece(pieceToMove[i]);
			}

			pieceToMove[i] = null;
			pieceToMoveStartSquare[i] = null;
			pieceToMoveFinishSquare[i] = null;
			pieceAdding[i]=null;
			pieceDeleting[i]=null;
		}
	}

	if (currTime - pieceToMoveStartTime > duration)
	{
		window.clearInterval(pieceToMoveIntervalId);
		pieceToMove.length=0;
		processTurn();	// next turn
	}
}

// ----------------------------------------------------------
// dragdrop.js
//
// library of functions relating to the drag/drop
// behaviour of the chess pieces
//
// see http://www.hunlock.com/examples/dragdrop.html
// for details of implementation
//
// for further reading, and possible improved drag/drop, see
// http://www.walterzorn.com/dragdrop/dragdrop_e.htm#config
// ----------------------------------------------------------
var savedTarget;
var dragDrop = new Object;
var isEnabled = new Boolean(true);		// set to true whilst server is thinking to 'disable board'

// to disable drag/drop
dragDrop.setEnabled = function(bool)
{
	isEnabled=bool;
}

dragDrop.getEnabled = function()
{
	return isEnabled;
}

dragDrop.moveHandler = function(e)
{
	if (e == null)
	{
		e = window.event;
	} 

	if (e.button<=1&&dragOK)
	{
		savedTarget.style.left=e.clientX-dragXoffset+'px';
		savedTarget.style.top=e.clientY-dragYoffset+'px';
		document.getElementById("inputsquareto").value = board.getSquareForTarget(savedTarget).toString();		
		return false;
	}
}

dragDrop.cleanup = function(e)
{
	document.onmousemove=null;
	document.onmouseup=null;
	savedTarget.style.cursor=orgCursor;
	dragOK=false;

	// snap piece to square
	if (e == null)
	{
		e = window.event;
		htype='move';
	} 

	var target = e.target != null ? e.target : e.srcElement;
	if (savedTarget.id.substring(0,6)=="piece_")
	{
		board.snapToSquare(savedTarget);
	}
	
	// finish user move
	board.finishmove();
}

dragDrop.dragHandler = function(e)
{
	var htype='-moz-grabbing';
	
	if (e == null)
	{
		e = window.event;
		htype='move';
	} 

	var target = e.target != null ? e.target : e.srcElement;

	orgCursor=target.style.cursor;

	if (target.id.substring(0,6)=="piece_")
	{
		// only allow drag/drop if 'enabled'
		if (dragDrop.getEnabled()==false)
			return false;
		else
			dragDrop.setEnabled(false);

		savedTarget=target;       
		target.style.cursor=htype;
		dragOK=true;
		dragXoffset=e.clientX-parseInt(target.style.left);
		dragYoffset=e.clientY-parseInt(target.style.top);
		document.onmousemove=dragDrop.moveHandler;
		document.onmouseup=dragDrop.cleanup;
		document.getElementById("inputsquarefrom").value = board.getSquare(e).toString();
		return false;
	}
}

// -------------------------------------------------
// servergateway.js
//
// Server Gateway object
// 
// Purpose: Standard interface to server code via
//          AJAX / HttpRequest object.
//          This object 
//          only 1 board required, therefore no need
//          for class
// -------------------------------------------------
<!-- AJAX HttpRequest object -->
if( !window.XMLHttpRequest ) XMLHttpRequest = function()
{
	try{ return new ActiveXObject("Msxml2.XMLHTTP.6.0") }catch(e){}
	try{ return new ActiveXObject("Msxml2.XMLHTTP.3.0") }catch(e){}
	try{ return new ActiveXObject("Msxml2.XMLHTTP") }catch(e){}
	try{ return new ActiveXObject("Microsoft.XMLHTTP") }catch(e){}
	throw new Error("Could not find an XMLHttpRequest alternative.")
};

// serverGateway object
var serverGateway = new Object;

// Parameters for a POST request must be encoded for use in a URL and separated with an ampersand.
// the parameters are subsequently passed as an argument to the HttpRequest.send() method.
serverGateway.addPostParams = function(oForm)
{
	var params = "";
	var delim = "";
	
	for (var i=0; i < oForm.length; i++)
	{
		params += delim + oForm.elements[i].id + "=" + oForm.elements[i].value;
		delim = "&";
	}
	return params;
}

// get computer move using AJAX
serverGateway.send = function()
{
	board.showwaitgif(document.getElementById("inputactivecolor").value);
	var oRequest = new XMLHttpRequest();
	
	// get form parameters
	var params = serverGateway.addPostParams(document.forms[0]);	
	
	// this code block prevents error:
	// uncaught exception: Permission denied to call method XMLHttpRequest.open
	try {
    	netscape.security.PrivilegeManager.enablePrivilege("UniversalBrowserRead"); }
	catch (e) {}
	
	oRequest.open ("post", "http://chess.fishysplash.com/chess.php", true);	// run asynchronously
	
	//Send the proper header information along with the request
	oRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	oRequest.setRequestHeader("Content-length", params.length);
	oRequest.setRequestHeader("Connection", "close");	

	oRequest.onreadystatechange = function()
	{
		// this code block prevents error:
		// uncaught exception: Permission denied to call method XMLHttpRequest.open
		try {
    		netscape.security.PrivilegeManager.enablePrivilege("UniversalBrowserRead"); }
		catch (e) {}

		if (oRequest.readyState==4)
		{
			var xmlResults = oRequest.responseXML.documentElement;

			//process results
			document.getElementById("inputnewgame").value="";
			document.getElementById("inputcode").value=
				xmlResults.getElementsByTagName('code')[0].childNodes[0].nodeValue;
			document.getElementById("inputstatus").value=
				xmlResults.getElementsByTagName('status')[0].childNodes[0].nodeValue;			
			document.getElementById("inputmove").value=
				xmlResults.getElementsByTagName('move')[0].childNodes[0].nodeValue;			
			document.getElementById("inputmoveui").value=
				xmlResults.getElementsByTagName('move_ui')[0].childNodes[0].nodeValue;						
			document.getElementById("inputpieceplacement").value=
				xmlResults.getElementsByTagName('piece_placement')[0].childNodes[0].nodeValue;			
			document.getElementById("inputcastleavailability").value=
				xmlResults.getElementsByTagName('castle_availability')[0].childNodes[0].nodeValue;						
			document.getElementById("inputenpassant").value=
				xmlResults.getElementsByTagName('enpassant')[0].childNodes[0].nodeValue;						
			document.getElementById("inputactivecolor").value=
				xmlResults.getElementsByTagName('active_color')[0].childNodes[0].nodeValue;						
			document.getElementById("inputhalfmoveclock").value=
				xmlResults.getElementsByTagName('half_move_clock')[0].childNodes[0].nodeValue;						
			document.getElementById("inputfullmovenumber").value=
				xmlResults.getElementsByTagName('full_move_number')[0].childNodes[0].nodeValue;						
			document.getElementById("inputsquarefrom").value="";
			document.getElementById("inputsquareto").value="";
			document.getElementById("inputnewgame").value="0";
			var nextMoveUI = xmlResults.getElementsByTagName('move_ui')[0].childNodes[0].nodeValue;
	
			// set the status bar text
			window.status = document.getElementById("inputstatus").value;
			
			board.hidewaitgif();
			dragDrop.setEnabled(true);
			
			if (document.getElementById("inputcode").value=="0")
			{
				// perform move[s] if not invalid
				setUIMoveText(nextMoveUI);
			}
		}
	}
	oRequest.send(params);	// must include the form parameters for POST request.
}
