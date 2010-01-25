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
  <title><?php print $title; ?></title>
  <?php if (theme_get_setting('toggle_favicon')): ?>
  <link rel="shortcut icon" href="<?php print check_url(theme_get_setting('favicon')); ?>" type="image/x-icon" />
  <?php endif; ?>
  <style type="text/css">
    #loading-mask {
      position: absolute;
      left: 0;
      top: 0;
      width: 100%;
      height: 100%;
      z-index: 20000;
      background-color: white;
    }
    #loading {
      position: absolute;
      left: 40%;
      top: 40%;
      padding: 2px;
      z-index: 20001;
      height: auto;
    }
    #loading a {
      color: #225588;
    }
    #loading .loading-indicator {
      background: white;
      color: #444;
      font: bold 13px tahoma, arial, helvetica;
      padding: 10px;
      margin: 0;
      height: auto;
    }
    #loading-msg {
      font: normal 10px arial, tahoma, sans-serif;
    }
  </style>

  <link rel="stylesheet" type="text/css" href="<?php print $extjs_path .'/resources/css/ext-all.css'; ?>" />
  <link rel="stylesheet" type="text/css" href="<?php print $extjs_path .'/resources/css/xtheme-gray.css'; ?>" />
  <link rel="stylesheet" type="text/css" href="<?php print $module_path .'/browser/fileuploadfield.css'; ?>" />
  <link rel="stylesheet" type="text/css" href="<?php print $module_path .'/browser/browser.css'; ?>" />
</head>
<body>
<div id="loading-mask"></div>
<div id="loading">
  <div class="loading-indicator">
    <img src="<?php print $module_path .'/images/loading.gif'; ?>" width="32" height="32" style="margin-right:8px;float:left;vertical-align:top;"/><?php print $module_info; ?>
    <br /><span id="loading-msg">Initializing...</span>
  </div>
</div>

<div id="header">
<h1><?php print $header; ?></h1>
<div id="search-box" class="x-normal-editor"><input type="text" id="search" /></div>
</div>

<script type="text/javascript" src="<?php print $extjs_path .'/adapter/ext/ext-base.js'; ?>"></script>
<script type="text/javascript" src="<?php print $extjs_path .'/ext-all.js'; ?>"></script>
<?php if ($locale_path): ?>
<script type="text/javascript" src="<?php print $locale_path; ?>"></script>
<?php endif; ?>
<script type="text/javascript" src="<?php print $module_path .'/browser/FileUploadField.js'; ?>"></script>
<script type="text/javascript" src="<?php print $module_path .'/browser/browser.js'; ?>"></script>

<script type="text/javascript">
  Ext.BLANK_IMAGE_URL = '<?php print $extjs_path .'/resources/images/default/s.gif'; ?>';
  AlfrescoBrowser.Settings = {
    'homeRef':'<?php print $home_ref; ?>',
    'homeText':'<?php print $home_text; ?>',
    'modulePath':'<?php print $module_path; ?>',
    'serviceTreeUrl':'<?php print $service_tree; ?>',
    'serviceGridUrl':'<?php print $service_grid; ?>',
    'serviceDownloadUrl':'<?php print $service_dl; ?>',
    'serviceUploadUrl':'<?php print $service_ul; ?>',
    'queryLimit':<?php print $query_limit; ?>
  };
</script>
</body>
</html>
