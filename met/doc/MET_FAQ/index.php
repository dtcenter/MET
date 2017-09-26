<?php
   # Set the page title here.
   $title = "MET FAQ";
   include ($_SERVER['DOCUMENT_ROOT'] . "/includes/begin_dtc.php");
?>

<div id="subheader"><!-- begin subheader -->
   <?php include ($_SERVER['DOCUMENT_ROOT'] . "/met/users/includes/subheader_met.php");?>
</div><!-- end subheader -->

<!-- Begin main page content -->
<div id="breadcrumb_nav"><!-- begin breadcrumb_nav -->
   <ul>
      <li>You are here: <a href="/index.php">DTC </a></li>

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

<?php
   $DOCUMENT_ROOT = $_SERVER['DOCUMENT_ROOT'];
   
   $absolute_faq_URL = "/met/users/support/faqs/";
   $faq_URL = $DOCUMENT_ROOT . $absolute_faq_URL;
   include ($faq_URL . "includes/sub_navigation.php");
   
   $tool_name = 'first_page';
   $cur_page_arg = $tool_name;
   $next_url = get_url_for_navigation($cur_page_arg, 'next');
   $prev_url = 'None';
   if ($prev_url != 'None') {
      $prev_button = '<a href="' . $absolute_faq_URL . $prev_url . '"><img src="/met/users/images/bck.gif"></a>';
      $prev_next_buttons = $prev_next_buttons . $prev_button;
   }
   if ($next_url != 'None') {
      $next_button = '<a href="' . $absolute_faq_URL . $next_url . '"><img src="/met/users/images/fwd.gif"></a>';
      $prev_next_buttons = $prev_next_buttons . $next_button;
   }
?>

<div id="column_pagenav"><!-- begin column_pagenav -->
   <div id="pagenav"><!-- begin pagenav -->
      <div class="suckerdiv"><!-- begin suckerdiv -->
         <?php include ($_SERVER['DOCUMENT_ROOT'] . "/met/users/includes/sidenav.html");?>
      </div><!-- end suckerdiv -->
   </div><!-- end pagenav -->
</div><!-- end column_pagenav -->

<div id="wrap_pagecontent"><!-- begin wrap_pagecontent -->
   <div id="column_pagecontent"><!-- begin column_pagecontent -->

      <div id="pagefeature"><!-- begin pagefeature -->
         <div id="headingdiv"><h1>Frequently Asked Questions</h1></div>
      </div><!-- end pagefeature -->
      <div id="navigationdiv" style="position: relative; float:right;">
         <?php print $prev_next_buttons; ?>
      </div>

      <div>
      <?php include "MET_FAQ_LINKS_MASTER.xhtml"; ?>
      </div>

      <div id="navigationdiv" style="float:right;">
         <?php print $prev_next_buttons; ?>
      </div>

      <div>
      <p>
      <div id="headingdiv"><h1>Old Frequently Asked Questions</h1></div>
         <p><strong>Q:</strong> Why was the MET written largely in C++ instead of FORTRAN?<br>
         <strong>A:</strong> MET relies upon the object-oriented aspects of C++.</p>

         <p><strong>Q:</strong> Why is PREPBUFR used?<br>
         <strong>A:</strong> The goal was to initially replicate the capabilities of other existing verification packages and make these capabilities available to both the DTC and the public.</p>

         <p><strong>Q:</strong> Why is GRIB used?<br>
         <strong>A:</strong> Forecast data from both WRF cores can be processed into GRIB format, and it is a commonly accepted output format for many NWP models.</p>

         <p><strong>Q:</strong> Is GRIB2 supported?<br>
         <strong>A:</strong> Yes, as of version 5.0.</p>

         <p><strong>Q:</strong> How does MET differ from the previously mentioned existing verification packages?<br>
         <strong>A:</strong> MET is an actively maintained, evolving software package that is being made freely available to the public through controlled version releases.</p>

         <p><strong>Q:</strong> How does the MODE tool differ from the Grid-Stat tool?<br>
         <strong>A:</strong> They offer different ways of viewing verification.  The Grid-Stat tool provides traditional verification statistics, while MODE provides specialized spatial statistics.</p>

         <p><strong>Q:</strong> Will the MET work on data in native model coordinates?<br>
         <strong>A:</strong> No - it will not.  In the future, we may add options to allow additional model grid coordinate systems.</p>

         <p><strong>Q:</strong> How do I get help if my questions are not answered in the Users Guide?<br>
         <strong>A:</strong> First, refer to the documentation on this website.  If that doesn't answer your question, then email: met_help@ucar.edu.</p>

         <p><strong>Q:</strong> Where are the graphics?<br>
         <strong>A:</strong> Currently, very few graphics are included.  Further graphics support will be made available in the future on the MET website.</p>

         <p><strong>Q:</strong> How do I reference the MET Users Guide in publications?<br>
         <strong>A:</strong> Please reference the MET Users Guide (including the version number) as follows:<br>
            <em>Developmental Testbed Center, 2008:<br>
            MET: Version 1.1 Model Evaluation Tools Users Guide.<br>
            Available at http://www.dtcenter.org/met/users/docs/overview.php.<br>
            168 pp.</em></p>
      </div>

   </div><!-- end column_pagecontent -->

   <div id="column_subcontent_wrap"><!-- begin column_subcontent_wrap -->
      <?php include ($_SERVER['DOCUMENT_ROOT'] . "/met/users/includes/sidenav_met.php");?>
      <!-- ?php include ($_SERVER['DOCUMENT_ROOT'] . "/met/users/support/faqs/includes/sidenav_faq.html"); ? -->
   </div><!-- end column_subcontent_wrap -->

</div><!-- end wrap_pagecontent -->

<br class="clearboth"/>

<!--end main page content-->
<?php include ($_SERVER['DOCUMENT_ROOT'] . "/includes/end.php");?>
