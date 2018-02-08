<?php
//still need to add $ip and $username = $clients[$ip][1] once done troubleshooting;
include_once 'db_credentials.php';
include_once 'utilityFunctions.php';
include_once 'groupFunctions.php';
include_once 'whiteboardFunctions.php';



function updateFlashCards($connection, $ip, $clients, $groupID, $sock) {
  $connection =  new mysqli(DB_Server, DB_User, DB_Pass, DB_Name);
  // Check connection
  if ($connection->connect_error)
    die("Connection failed: " . $connection->connect_error);
  //SQL Commands
  $flashGroupID = "$groupID" . "FC";


  //RETURN FRONT SIDE
  $return_FlashCards = "SELECT id, side1
                        FROM $flashGroupID
                        WHERE id IS NOT NULL";
  $resultFlashCards = mysqli_query($connection, $return_FlashCards); //runs and stores results of all flash cards in the group currently.
  $num_FlashCards = $resultFlashCards->num_rows;//stores number of flashcards
  echo "We are writing to current user joining group: $ip who's socket should be $sock \n";
  echo "Debugging: What is the number of Flashcards: $num_FlashCards \n";
  for($n_FlashCards = $num_FlashCards; $n_FlashCards > 0; $n_FlashCards = $n_FlashCards - 1){ //For loop that iterates through lists of flashcards to writeback to client.
    $row=mysqli_fetch_array($resultFlashCards); //Fetches first flash card into array
    $newID = $row[0] -1;
    $FlashCards = "$newID $row[1]"; //Stores side1 and side2 into variable
    echo "Debugging: We are writing $FlashCards to ip $ip with socket $sock \n";
    $message = "FCFT$FlashCards"; //Appends CODE FLSH to username
    $messageSize = str_pad((string)strlen($message), 5, "0", STR_PAD_LEFT); //Pads left of code with length of string so client knows how much to read
    fwrite($sock,"{$messageSize}{$message}"); //Writes back to client.
    echo "Debugging: Client should be receiving: {$messageSize}{$message} \n";
  } //closes for loop

  //RETURN BACK SIDE
  $return_FlashCards = "SELECT id, side2
                        FROM $flashGroupID
                        WHERE id IS NOT NULL";
  $resultFlashCards = mysqli_query($connection, $return_FlashCards); //runs and stores results of all flash cards in the group currently.
  $num_FlashCards = $resultFlashCards->num_rows;//stores number of flashcards
  echo "We are writing to current user joining group: $ip who's socket should be $sock \n";
  echo "Debugging: What is the number of Flashcards: $num_FlashCards \n";
  for($n_FlashCards = $num_FlashCards; $n_FlashCards > 0; $n_FlashCards = $n_FlashCards - 1){ //For loop that iterates through lists of flashcards to writeback to client.
    $row=mysqli_fetch_array($resultFlashCards); //Fetches first flash card into array
    $newID = $row[0] -1;
    $FlashCards = "$newID $row[1]"; //Stores side1 and side2 into variable
    echo "Debugging: We are writing $FlashCards to ip $ip with socket $sock \n";
    $message = "FCBK$FlashCards"; //Appends CODE FLSH to username
    $messageSize = str_pad((string)strlen($message), 5, "0", STR_PAD_LEFT); //Pads left of code with length of string so client knows how much to read
    fwrite($sock,"{$messageSize}{$message}"); //Writes back to client.
    echo "Debugging: Client should be receiving: {$messageSize}{$message} \n";
  } //closes for loop
}//Close function



function addToSide1($groupID, $num, $message, $ip, $clients, $sock) {
  $connection =  new mysqli(DB_Server, DB_User, DB_Pass, DB_Name);
  $user = $clients[$ip][1];
  $num = $num + 1;
  // Check connection
  if ($connection->connect_error)
    die("Connection failed: " . $connection->connect_error);

  $user = mysqli_real_escape_string($connection, $user);
  $message = mysqli_real_escape_string($connection, $message);
  $groupID = mysqli_real_escape_string($connection, $groupID);
  $flashGroupID = "$groupID" . "FC";

  $return_ipList = "SELECT ipAddress FROM $groupID WHERE ipAddress IS NOT NULL";
  $resultIP = mysqli_query($connection, $return_ipList); //Returns list of current IP addresses i.e. current user list connected.
  $num_ip = $resultIP->num_rows; //Stores number of people currently connected for while loop iteration.

  $query = "INSERT INTO $flashGroupID (user, side1) VALUES ('$user', '$message')";

// Check to see if the id for this card exists already
  $check_card = "SELECT * FROM $flashGroupID WHERE (id='$num')";

  if ($stmt = mysqli_prepare($connection, $check_card)){
    //Execute query
    mysqli_stmt_execute($stmt);
    //Store result of query
    mysqli_stmt_store_result($stmt);
    $card_exists = mysqli_stmt_num_rows($stmt);
    //Close statement
    mysqli_stmt_close($stmt);
  }

  if ($card_exists > 0){
    echo "Card exists already ";
    $update = "UPDATE $flashGroupID SET user= '$user', side1='$message' WHERE (id='$num')";
    mysqli_query($connection, $update);
    $NewID = "SELECT id FROM $flashGroupID WHERE (user='$user' AND side1='$message')";
    $result = mysqli_query($connection, $NewID);
    $obj = $result->fetch_object();
    $returnID = $obj->id; // returnID == return value of NewID
    $returnID = $returnID -1;
    $clientMessage = "SUCC{$returnID}";
    $messageSize = str_pad((string)strlen($clientMessage), 5, "0", STR_PAD_LEFT); //might need tuning
    fwrite($sock, "{$messageSize}{$clientMessage}"); //writes to the socket

    while($num_ip > 0) { //Loops through each active client, printing out the most recent flash card edited in order to update ui
      $rowIP = mysqli_fetch_array($resultIP); //Fetches first IP as an array.
      //echo "Debugging: This is keyIP we're using to index: $rowIP[0] \n";
      $keyIP = $rowIP[0]; //Stores the key IP address:Port for use of $clients dict.
      if ($keyIP != $ip){
        $keySock = $clients[$keyIP][0]; //Uses the above to access the socket client of the IP address to write back to.
        //echo "Debugging: This is keySock we're writing to: $keySock \n";
        $FlashCards = "$returnID $message"; //Stores side1 and side2 into variable
        echo "Debugging: We are writing $FlashCards to ip $ip with socket $sock \n";
        $clientMessage = "FCFT$FlashCards"; //Appends CODE FLSH to username
        $messageSize = str_pad((string)strlen($clientMessage), 5, "0", STR_PAD_LEFT); //Pads left of code with length of string so client knows how much to read
        fwrite($keySock,"{$messageSize}{$clientMessage}"); //Writes back to client.
        echo "Debugging: Client should be receiving: {$messageSize}{$clientMessage} \n";
      } //closes if statement
      $num_ip = $num_ip - 1; //Goes to next IP address/User in group
    }//end while Loops
  }//Closes outer if statement

  else{
    mysqli_query($connection, $query);
    $NewID = "SELECT id FROM $flashGroupID ORDER BY id DESC LIMIT 1";
    $result = mysqli_query($connection, $NewID);
    //echo "newID query has run. result is: $result\n\n";
    $obj = $result->fetch_object();
    $returnID = $obj->id;
    $returnID = $returnID -1;
    //echo "returnID is: $returnID\n\n";
    $clientMessage = "SUCC{$returnID}";
    //echo "clientMessage is: $clientMessage\n\n";
    $messageSize = str_pad((string)strlen($clientMessage), 5, "0", STR_PAD_LEFT); //might need tuning
    fwrite($sock, "{$messageSize}{$clientMessage}"); //writes to the socket

    while($num_ip > 0) { //Loops through each active client, printing out the most recent flash card edited in order to update ui
      $rowIP = mysqli_fetch_array($resultIP); //Fetches first IP as an array.
      //echo "Debugging: This is keyIP we're using to index: $rowIP[0] \n";
      $keyIP = $rowIP[0]; //Stores the key IP address:Port for use of $clients dict.
      if ($keyIP != $ip){
        $keySock = $clients[$keyIP][0]; //Uses the above to access the socket client of the IP address to write back to.
        //echo "Debugging: This is keySock we're writing to: $keySock \n";
        $FlashCards = "$returnID $message"; //Stores side1 and side2 into variable
        echo "Debugging: We are writing $FlashCards to ip $ip with socket $sock \n";
        $clientMessage = "FCFT$FlashCards"; //Appends CODE FLSH to username
        $messageSize = str_pad((string)strlen($clientMessage), 5, "0", STR_PAD_LEFT); //Pads left of code with length of string so client knows how much to read
        fwrite($sock,"{$messageSize}{$clientMessage}"); //Writes back to client.
        echo "Debugging: Client should be receiving: {$messageSize}{$clientMessage} \n";
      }// break in case current ip = client ip ; basically only update others, not current client
      $num_ip = $num_ip - 1; //Goes to next IP address/User in group
    }//end while Loops

    $connection->close();
  } // end else bracket
}//close function


function addToSide2($groupID, $num, $message, $ip, $clients, $sock) {
  $connection =  new mysqli(DB_Server, DB_User, DB_Pass, DB_Name);
  $user = $clients[$ip][1];
  $num = $num + 1;
  // Check connection
  if ($connection->connect_error)
    die("Connection failed: " . $connection->connect_error);

  $user = mysqli_real_escape_string($connection, $user);
  $message = mysqli_real_escape_string($connection, $message);
  $groupID = mysqli_real_escape_string($connection, $groupID);
  $flashGroupID = "$groupID" . "FC";

  $return_ipList = "SELECT ipAddress FROM $groupID WHERE ipAddress IS NOT NULL";
  $resultIP = mysqli_query($connection, $return_ipList); //Returns list of current IP addresses i.e. current user list connected.
  $num_ip = $resultIP->num_rows; //Stores number of people currently connected for while loop iteration.

  $query = "INSERT INTO $flashGroupID (user, side2) VALUES ('$user', '$message')";

  // Check to see if the id for this card exists already
  $check_card = "SELECT * FROM $flashGroupID WHERE (id='$num')";

  if ($stmt = mysqli_prepare($connection, $check_card)){
    //Execute query
    mysqli_stmt_execute($stmt);
    //Store result of query
    mysqli_stmt_store_result($stmt);
    $card_exists = mysqli_stmt_num_rows($stmt);
    //Close statement
    mysqli_stmt_close($stmt);
  }

  if ($card_exists > 0){
    echo "Card exists already ";
    $update = "UPDATE $flashGroupID SET user= '$user', side2='$message' WHERE (id='$num')";
    mysqli_query($connection, $update);
    $NewID = "SELECT id FROM $flashGroupID WHERE (user='$user' AND side2='$message')";
    $result = mysqli_query($connection, $NewID);
    $obj = $result->fetch_object();
    $returnID = $obj->id; // returnID == return value of NewID
    $returnID = $returnID -1;
    $clientMessage = "SUCC{$returnID}";
    $messageSize = str_pad((string)strlen($clientMessage), 5, "0", STR_PAD_LEFT); //might need tuning
    fwrite($sock, "{$messageSize}{$clientMessage}"); //writes to the socket

    while($num_ip > 0) { //Loops through each active client, printing out the most recent flash card edited in order to update ui
      $rowIP = mysqli_fetch_array($resultIP); //Fetches first IP as an array.
      //echo "Debugging: This is keyIP we're using to index: $rowIP[0] \n";
      $keyIP = $rowIP[0]; //Stores the key IP address:Port for use of $clients dict.
      if ($keyIP != $ip){
        $keySock = $clients[$keyIP][0]; //Uses the above to access the socket client of the IP address to write back to.
        //echo "Debugging: This is keySock we're writing to: $keySock \n";
        $FlashCards = "$returnID $message"; //Stores side1 and side2 into variable
        echo "Debugging: We are writing $FlashCards to ip $ip with socket $sock \n";
        $clientMessage = "FCBK$FlashCards"; //Appends CODE FLSH to username
        $messageSize = str_pad((string)strlen($clientMessage), 5, "0", STR_PAD_LEFT); //Pads left of code with length of string so client knows how much to read
        fwrite($keySock,"{$messageSize}{$clientMessage}"); //Writes back to client.
        echo "Debugging: Client should be receiving: {$messageSize}{$clientMessage} \n";
      } //closes if statement
      $num_ip = $num_ip - 1; //Goes to next IP address/User in group
    }//end while Loops
  }//end if statement

  else{
    mysqli_query($connection, $query);
    $NewID = "SELECT id FROM $flashGroupID ORDER BY id DESC LIMIT 1";
    $result = mysqli_query($connection, $NewID);
    //echo "newID query has run. result is: $result\n\n";
    $obj = $result->fetch_object();
    $returnID = $obj->id;
    $returnID = $returnID -1;
    //echo "returnID is: $returnID\n\n";
    $clientMessage = "SUCC{$returnID}";
    //echo "clientMessage is: $clientMessage\n\n";
    $messageSize = str_pad((string)strlen($clientMessage), 5, "0", STR_PAD_LEFT); //might need tuning
    fwrite($sock, "{$messageSize}{$clientMessage}"); //writes to the socket
    while($num_ip > 0) { //Loops through each active client, printing out the most recent flash card edited in order to update ui
      $rowIP = mysqli_fetch_array($resultIP); //Fetches first IP as an array.
      //echo "Debugging: This is keyIP we're using to index: $rowIP[0] \n";
      $keyIP = $rowIP[0]; //Stores the key IP address:Port for use of $clients dict.
      if ($keyIP != $ip){
        $keySock = $clients[$keyIP][0]; //Uses the above to access the socket client of the IP address to write back to.
        //echo "Debugging: This is keySock we're writing to: $keySock \n";
        $FlashCards = "$returnID $message"; //Stores side1 and side2 into variable
        echo "Debugging: We are writing $FlashCards to ip $ip with socket $sock \n";
        $clientMessage = "FCBK$FlashCards"; //Appends CODE FLSH to username
        $messageSize = str_pad((string)strlen($clientMessage), 5, "0", STR_PAD_LEFT); //Pads left of code with length of string so client knows how much to read
        fwrite($sock,"{$messageSize}{$clientMessage}"); //Writes back to client.
        echo "Debugging: Client should be receiving: {$messageSize}{$clientMessage} \n";
      }// break in case current ip = client ip ; basically only update others, not current client
      $num_ip = $num_ip - 1; //Goes to next IP address/User in group
    }//end while Loops
  }//End else statement
  $connection->close();

}// End function