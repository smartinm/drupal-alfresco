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
    <link type="text/css" rel="stylesheet" media="all" href="<?php print $path_url . '/ext/css/ext-all.css'; ?>" />
    <link type="text/css" rel="stylesheet" media="all" href="<?php print $path_url . '/ext/css/xtheme-gray.css'; ?>" />
    <link type="text/css" rel="stylesheet" media="all" href="<?php print $path_url . '/js/alfresco_browser.css'; ?>" />
    <script type="text/javascript" src="<?php print $base_url . '/misc/jquery.js'; ?>"></script>
    <script type="text/javascript" src="<?php print $path_url . '/ext/ext-jquery-adapter.js'; ?>"></script>
    <!-- <script type="text/javascript" src="<?php print $path_url . '/ext/ext-base.js'; ?>"></script>  -->
    <script type="text/javascript" src="<?php print $path_url . '/ext/ext-all.js'; ?>"></script>
    <script type="text/javascript" src="<?php print $path_url . '/js/alfresco_browser.js'; ?>"></script>
  </head>
  <body>

    <div id="header"><h1>Alfresco Browser</h1></div>

    <div style="display:none;">

        <!-- Start page content -->
        <div id="start-div">
          <h2>Welcome!</h2>
          <p>There are many sample layouts to choose from that should give you a good head start in building your own
          application layout.  Just like the combination examples, you can mix and match most layouts as
          needed, so don't be afraid to experiment!</p>
          <p>Select a layout from the tree to the left to begin.</p>
        </div>
    </div>

  </body>
</html>
