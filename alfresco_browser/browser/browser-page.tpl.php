<?php
// $Id: alfresco_browser-page.tpl.php 1090 2010-01-11 18:08:20Z semm $

/**
 * @file alfresco_browser-page.tpl.php
 * Default theme implementation for Alfresco Browser page.
 *
 * @see template_preprocess_alfresco_browser_page()
 */
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <?php if (theme_get_setting('toggle_favicon')): ?>
  <link rel="shortcut icon" href="<?php print check_url(theme_get_setting('favicon')); ?>" type="image/x-icon" />
  <?php endif; ?>
  <link rel="stylesheet" type="text/css" href="<?php print check_url($extjs_path .'/resources/css/ext-all.css'); ?>" />
  <link rel="stylesheet" type="text/css" href="<?php print check_url($extjs_path .'/resources/css/xtheme-gray.css'); ?>" />
  <link rel="stylesheet" type="text/css" href="<?php print check_url($module_path .'/browser/browser.css'); ?>" />
  <script type="text/javascript" src="<?php print check_url($extjs_path .'/adapter/ext/ext-base.js'); ?>"></script>
  <script type="text/javascript" src="<?php print check_url($extjs_path .'/ext-all.js'); ?>"></script>
  <?php if ($locale_path): ?>
  <script type="text/javascript" src="<?php print check_url($locale_path); ?>"></script>
  <?php endif; ?>
  <script type="text/javascript" src="<?php print check_url($module_path .'/browser/browser.js'); ?>"></script>
  <script type="text/javascript">
    Ext.BLANK_IMAGE_URL = '<?php print check_url($extjs_path .'/resources/images/default/s.gif'); ?>';
    AlfrescoBrowser.Settings = {
      'homeRef':'<?php print $home_ref; ?>',
      'homeText':'<?php print $home_text; ?>',
      'modulePath':'<?php print $module_path; ?>',
      'serviceTreeUrl':'<?php print $service_tree; ?>',
      'serviceGridUrl':'<?php print $service_grid; ?>',
      'serviceDownloadUrl':'<?php print $service_dl; ?>',
      'queryLimit':<?php print $query_limit; ?>
    };
  </script>
  <title><?php print $title; ?></title>
</head>
<body>
<div id="header">
<h1><?php print $header; ?></h1>
<div id="search-box" class="x-normal-editor"><input type="text" id="search" /></div>
</div>
</body>
</html>
