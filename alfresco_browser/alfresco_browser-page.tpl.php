<?php
// $Id$

/**
 * @file alfresco_browser-page.tpl.php
 * Default theme implementation for XXX.
 *
 * Available variables:
 * - $title: Top level node title.
 * - $head: Header tags.
 * - $language: Language code. e.g. "en" for english.
 * - $language_rtl: TRUE or FALSE depending on right to left language scripts.
 * - $base_url: URL to home page.
 * - $content: Nodes within the current outline rendered through
 *   book-node-export-html.tpl.php.
 *
 * @see template_preprocess_alfresco_browser_page()
 */
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="<?php print $language->language; ?>" xml:lang="<?php print $language->language; ?>">
<head>
<title><?php print $title; ?></title>
<?php print $head; ?>
<link type="text/css" rel="stylesheet" media="all" href="<?php print $module_path .'/ext/css/ext-all.css'; ?>" />
<link type="text/css" rel="stylesheet" media="all" href="<?php print $module_path .'/ext/css/xtheme-gray.css'; ?>" />
<link type="text/css" rel="stylesheet" media="all" href="<?php print $module_path .'/css/alfresco_browser.css'; ?>" />
<?php print $scripts; ?>
<script type="text/javascript">
<!--//--><![CDATA[//><!--
Ext.BLANK_IMAGE_URL = '<?php print $module_path .'/ext/images/default/s.gif'; ?>';
//--><!]]>
</script>
</head>
<body>
</body>
</html>
