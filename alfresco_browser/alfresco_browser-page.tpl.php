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
  <link rel="stylesheet" type="text/css" href="<?php print $module_path .'/ext/css/ext-all.css'; ?>" />
  <link rel="stylesheet" type="text/css" href="<?php print $module_path .'/ext/css/xtheme-gray.css'; ?>" />
  <link rel="stylesheet" type="text/css" href="<?php print $module_path .'/css/alfresco_browser.css'; ?>" />
  <script type="text/javascript" src="<?php print $module_path .'/ext/ext-base.js'; ?>"></script>
  <script type="text/javascript" src="<?php print $module_path .'/ext/ext-all-debug.js'; ?>"></script>
  <script type="text/javascript" src="<?php print $module_path .'/js/alfresco_browser.js'; ?>"></script>
  <script type="text/javascript">
  <!--//--><![CDATA[//><!--
    Ext.BLANK_IMAGE_URL = "<?php print $module_path .'/ext/images/default/s.gif'; ?>";
    AlfrescoBrowser.Settings = {
      'moduleUrl':'<?php print $module_path; ?>',
      'serviceTreeUrl':'<?php print url('alfresco/browser/service/tree.json'); ?>',
      'serviceGridUrl':'<?php print url('alfresco/browser/service/grid.json'); ?>',
      'serviceDownloadUrl':'<?php print url('alfresco/browser/service/download'); ?>'
    };
  //--><!]]>
  </script>
  <title><?php print $title; ?></title>
</head>
<body>
<div style="float:right;margin:5px;"><input type="text" id="search" /></div>
</body>
</html>
