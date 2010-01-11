<?php
// $Id$

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
  <link rel="stylesheet" type="text/css" href="<?php print $extjs_path .'/resources/css/ext-all.css'; ?>" />
  <link rel="stylesheet" type="text/css" href="<?php print $extjs_path .'/resources/css/xtheme-gray.css'; ?>" />
  <link rel="stylesheet" type="text/css" href="<?php print $module_path .'/css/alfresco_browser_page.css'; ?>" />
  <script type="text/javascript" src="<?php print $extjs_path .'/adapter/ext/ext-base.js'; ?>"></script>
  <script type="text/javascript" src="<?php print $extjs_path .'/ext-all.js'; ?>"></script>
  <!-- <script type="text/javascript" src="<?php print $extjs_path .'/source/locale/ext-lang-es.js'; ?>"></script>  -->
  <script type="text/javascript" src="<?php print $module_path .'/js/alfresco_browser_page.js'; ?>"></script>
  <script type="text/javascript">
    Ext.BLANK_IMAGE_URL = "<?php print $extjs_path .'/resources/images/default/s.gif'; ?>";
    AlfrescoBrowser.Settings = {
      'moduleUrl':'<?php print $module_path; ?>',
      'serviceTreeUrl':'<?php print url('alfresco/browser/service/tree.json'); ?>',
      'serviceGridUrl':'<?php print url('alfresco/browser/service/grid.json'); ?>',
      'serviceDownloadUrl':'<?php print url('alfresco/browser/service/download'); ?>'
    };
  </script>
  <title><?php print $title; ?></title>
</head>
<body>
<div id="header">
<h1><?php print t('Alfresco Browser for Drupal'); ?></h1>
<div id="search-box" class="x-normal-editor"><input type="text" id="search" /></div>
</div>
</body>
</html>
