<?php
   # Set the page title here.
   $title = "MET FAQs";
   $DOCUMENT_ROOT = $_SERVER['DOCUMENT_ROOT'];
   include ($DOCUMENT_ROOT . "/includes/begin_dtc.php");
   
   $absolute_faq_URL = "/met/users/support/faqs/";
   $faq_URL = $DOCUMENT_ROOT . $absolute_faq_URL;
   include ($faq_URL . "includes/sub_navigation.php");
   include ("$DOCUMENT_ROOT/met/users/includes/tutorial_style.html");

   if(!isset($_GET["name"])) {
      $tool_name = 'first_page';
      $include_URL = get_url_for_navigation($tool_name, '');
   }
   else {
      $tool_name = $_GET['name'];
      $sub_category = $_GET['category'];
      $include_URL = "$tool_name/$tool_name" . "_" . "$sub_category.xhtml";
   }
   if (!file_exists($include_URL)) {
      if($tool_name == "troubleshooting" and $sub_category == "stat_analysis_slow" ) {
         $include_URL = "$tool_name/trouble_shooting_$sub_category.xhtml";
      }
      else if($tool_name == "regrid_data_plane" and $sub_category == "wgrib2" ) {
         $include_URL = "$tool_name/regrid_with_wgrib2.xhtml";
      }
   }
   if (!file_exists($include_URL)) {
      $include_URL = "MET_FAQ_LINKS_MASTER.xhtml";
   }
   
   
   $cur_page_arg = $tool_name;
   if ($sub_category) {
      $cur_page_arg = $tool_name . "_" . $sub_category;
   }
   $prev_url = get_url_for_navigation($cur_page_arg, 'prev');
   $next_url = get_url_for_navigation($cur_page_arg, 'next');
   $prev_next_buttons = '';
   if ($prev_url != 'None') {
      $prev_button = '<a href="' . $absolute_faq_URL . $prev_url . '"><img src="/met/users/images/bck.gif"></a>';
      $prev_next_buttons = $prev_next_buttons . $prev_button;
   }
   if ($next_url != 'None') {
      $next_button = '<a href="' . $absolute_faq_URL . $next_url . '"><img src="/met/users/images/fwd.gif"></a>';
      $prev_next_buttons = $prev_next_buttons . $next_button;
   }
?>

   
<div id="subheader"><!-- begin subheader -->
   <?php include ("$DOCUMENT_ROOT/met/users/includes/subheader_met.php");?>
</div><!-- end subheader -->

<!-- Begin main page content -->
<div id="breadcrumb_nav"><!-- begin breadcrumb_nav -->
   <ul>
      <li>You are here: <a href="/index.php">DTC &bull; </a></li>
      <li><a href="/met/users/index.php">MET FAQs</a></li>

      <?php
         # Set the current section for breadcrumb location.
         while (list($key, $val) = each($sections) ) {
            echo "  <li><a ";
            if ($val['name'] == $current_section) {
               echo 'class="current" ';
               echo 'href="'  . $val['uri'] . '">' . $val['name'] . "</a> $current_section</li>\n";
            }
         }
      ?>

      <li>&bull; <i><?php echo "$title";?></i></li>
   </ul>
</div><!-- end breadcrumb_nav -->

<div id="column_pagenav"><!-- begin column_pagenav -->
   <div id="pagenav"><!-- begin pagenav -->
      <div class="suckerdiv"><!-- begin suckerdiv -->
         <?php include ($_SERVER['DOCUMENT_ROOT'] . $absolute_faq_URL . "includes/sidenav_faq.html");?>
      </div><!-- end suckerdiv -->
   </div><!-- end pagenav -->
</div><!-- end column_pagenav -->

<div id="wrap_pagecontent"><!-- begin wrap_pagecontent -->
   <div id="navigationdiv" style="position: relative; top: 4.4em;">
      <?php print $prev_next_buttons; ?>
   </div>
   <div id="column_pagecontent"><!-- begin column_pagecontent -->
      <div id="pagefeature"><!-- begin pagefeature -->
         <div id="headingdiv"><h1>MET FAQ</h1></div>
      </div><!-- end pagefeature -->
      <!-- &nbsp; -->
      <?php include "$include_URL"; ?>
   </div><!-- end column_pagecontent -->
   
   <div id="navigationdiv">
      <?php print $prev_next_buttons; ?>
   </div>

</div><!-- end wrap_pagecontent -->

<br class="clearboth"/>

<!--end main page content-->
<?php include ($_SERVER['DOCUMENT_ROOT'] . "/includes/end.php");?>
