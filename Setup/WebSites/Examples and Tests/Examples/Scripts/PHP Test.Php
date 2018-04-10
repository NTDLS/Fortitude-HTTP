<!--#include virtual="/@PageTop.Html"-->

<h2>PHP v<?php echo phpversion();?> Test</h2>

<?php
error_reporting(0); // Turn off all error reporting

$dofb = strtotime ("04 June 2000");
$age = strtotime ("now")-$dofb;
$age_in_years = $age/31557600;

echo "<!--#echo var=\"SERVER_SOFTWARE\"--> has been in development for " . round($age_in_years, 8) . " years<br /><br />";

//phpinfo();

echo "<b>Request Details (PHP v" . phpversion() . "):</b><br />";
echo "<blockquote>";
  echo "<b>path_info</b>: " .                $_SERVER["PATH_INFO"] . "<br />";
  echo "<b>script_filename</b>: " .          $_SERVER["SCRIPT_FILENAME"] . "<br />";
  echo "<b>request_method</b>: " .           $_SERVER["REQUEST_METHOD"] . "<br />";
  echo "<b>query_string</b>: " .             $_SERVER["QUERY_STRING"] . "<br />";
  echo "<b>http_referer</b>: " .             $_SERVER["HTTP_REFERER"] . "<br />";
  echo "<b>http_cookie</b>: " .              $_SERVER["HTTP_COOKIE"] . "<br />";
  echo "<b>http_connection</b>: " .          $_SERVER["HTTP_CONNECTION"] . "<br />";
  echo "<b>server_software</b>: " .          $_SERVER["SERVER_SOFTWARE"] . "<br />";
  echo "<b>server_protocol</b>: " .          $_SERVER["SERVER_PROTOCOL"] . "<br />";
  echo "<b>gateway_interface</b>: " .        $_SERVER["GATEWAY_INTERFACE"] . "<br />";
  echo "<b>http_host</b>: " .                $_SERVER["HTTP_HOST"] . "<br />";
  echo "<b>server_port</b>: " .              $_SERVER["SERVER_PORT"] . "<br />";
  echo "<b>server_name</b>: " .              $_SERVER["SERVER_NAME"] . "<br />";
  echo "<b>path_translated</b>: " .          $_SERVER["PATH_TRANSLATED"] . "<br />";
  echo "<b>remote_addr</b>: " .              $_SERVER["REMOTE_ADDR"] . "<br />";
  echo "<b>script_name</b>: " .              $_SERVER["SCRIPT_NAME"] . "<br />";
  echo "<b>http_accept</b>: " .              $_SERVER["HTTP_ACCEPT"] . "<br />";
  echo "<b>http_accept_language</b>: " .     $_SERVER["HTTP_ACCEPT_LANGUAGE"] . "<br />";
  echo "<b>http_user_agent</b>: " .          $_SERVER["HTTP_USER_AGENT"] . "<br />";
  echo "<b>document_root</b>: " .            $_SERVER["DOCUMENT_ROOT"] . "<br />";
  echo "<b>path</b>: " .                     $_SERVER["PATH"] . "<br />";
  echo "<b>pathext</b>: " .                  $_SERVER["PATHEXT"] . "<br />";
  echo "<b>number_of_processors</b>: " .     $_SERVER["NUMBER_OF_PROCESSORS"] . "<br />";
  echo "<b>processor_architecture</b>: " .   $_SERVER["PROCESSOR_ARCHITECTURE"] . "<br />";
  echo "<b>processor_identifier</b>: " .     $_SERVER["PROCESSOR_IDENTIFIER"] . "<br />";
  echo "<b>processor_level</b>: " .          $_SERVER["PROCESSOR_LEVEL"] . "<br />";
  echo "<b>processor_revision</b>: " .       $_SERVER["PROCESSOR_REVISION"] . "<br />";
  echo "<b>os</b>: " .                       $_SERVER["OS"] . "<br />";
echo "</blockquote>";
?>

<!--#include virtual="/@PageBottom.Html"-->
