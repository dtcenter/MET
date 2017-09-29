<?php

$local_debug = false;
$local_debug_all = false;
$arg_key = 'debug';
if (array_key_exists($arg_key, $_GET)) $local_debug = true;
if (array_key_exists($arg_key ."_all", $_GET)) $local_debug_all = true;

$navigation_array = array();
$nav_search_list = array ('name', '=', 'category', '&');
$nav_replace_list = array ('', '', '', '_');
$navigation_array_key = "navigation_array_key";

function get_url_for_navigation($cur_page_arg, $direction) {
  global $navigation_array, $nav_search_list, $nav_replace_list;
  $nav_key = trim(str_replace($nav_search_list, $nav_replace_list, $cur_page_arg));
  if (isset($direction) && $direction != "") {
    $nav_key = $nav_key . "." . $direction;
  }
  if ($local_debug) {
    print '    nav_key: '. $nav_key . ' value: ' . $navigation_array[$nav_key];
  }
  return $navigation_array[$nav_key];
}

function read_navigation_URLS() {
  global $navigation_array, $local_debug, $local_debug_all;
  
  $findme   = ':';
  //echo getcwd() . " --------------- \n";

  $handle = fopen("includes/navigations.cfg", "r");
  if ($handle == false) $handle = fopen("navigations.cfg", "r");
  if ($handle) {
    while (($line = fgets($handle)) !== false) {
      // process the line read.
      $pos = strpos($line, $findme);
      if ($pos !== false) {
        $nav_key = trim(substr($line, 0, $pos ));
        $nav_value = trim(substr($line, $pos+1 ));
            
        $navigation_array[$nav_key] = $nav_value;
        if ($local_debug_all) {
          print "line: [" . $line . "] key = [" . $nav_key . "], value:    [". $nav_value . "]<br>";
        }
      }
    }
  
    fclose($handle);
    if ($local_debug_all) print_r($navigation_array);
  } else {
    // error opening the file.
    print "  Can not find navigations.prop<br>";
  }
}


session_start();

$navigation_array_in_session = null;
if (array_key_exists($navigation_array_key, $_SESSION)) $navigation_array_in_session = $_SESSION[$navigation_array_key];
if ($navigation_array_in_session == null || array_key_exists('reload', $_GET)) {
  if ($local_debug) {
    print "<p>===================================================================<br>";
    print "   DEBUG message: Read navigation URLs";
    print "<br>===================================================================</p>";
  }
  read_navigation_URLS();
  $_SESSION[$navigation_array_key] = $navigation_array;
} else {
  $navigation_array = $navigation_array_in_session;
}

if ($local_debug) {
  $nav_url_args = 'name=modis_regrid&category=output';
  $new_url_next = get_url_for_navigation($nav_url_args, 'next');
  $new_url_prev = get_url_for_navigation($nav_url_args, 'prev');
  
  print "<p>===================================================================<br>";
  print "   from " . $nav_url_args . " to [" . $new_url_next . "] or [" . $new_url_prev;
  print "<br>===================================================================</p>";
}
?>