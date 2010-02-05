<?php
// $Id$

/**
 *
 */
class alfresco_handler_field_datetime extends views_handler_field_date {

  function render($values) {

    if (preg_match('/^(\d\d\d\d)-(\d\d)-(\d\d) (\d\d:\d\d:\d\d)$/', $value)) {
      if(strtotime($value)) {
        $value = strtotime($value);
        $values->{$this->field_alias} = $value;
      }
    }

    // Convert the stored datetime text string into a unix timestamp, as this is what the
    // regular unix timestamp formatter is expecting. This function will be called for each datetime
    // field in the view, so if a field is repeated it must only be converted once.
    static $converted = array();
    //if (!$converted[$this->field_alias]) {
      $values->{$this->field_alias} = strToTime($values->{$this->field_alias});
      $converted[$this->field_alias] = TRUE;
    //}

    // Render the date using the normal date handler
    return parent::render($values);
  }

  function render($values) {

    $value = $values->{$this->field_alias};
    $format = $this->options['date_format'];
    if (in_array($format, array('custom', 'raw time ago', 'time ago', 'raw time span', 'time span'))) {
      $custom_format = $this->options['custom_date_format'];
    }

    if (!$value) {
      return theme('views_nodate');
    }
    else {
      $time_diff = time() - $value; // will be positive for a datetime in the past (ago), and negative for a datetime in the future (hence)
      switch ($format) {
        case 'raw time ago':
          return format_interval($time_diff, is_numeric($custom_format) ? $custom_format : 2);
        case 'time ago':
          return t('%time ago', array('%time' => format_interval($time_diff, is_numeric($custom_format) ? $custom_format : 2)));
        case 'raw time span':
          return ($time_diff < 0 ? '-' : '') . format_interval(abs($time_diff), is_numeric($custom_format) ? $custom_format : 2);
        case 'time span':
          return t(($time_diff < 0 ? '%time hence' : '%time ago'), array('%time' => format_interval(abs($time_diff), is_numeric($custom_format) ? $custom_format : 2)));
        case 'custom':
          return format_date($value, $format, $custom_format);
        default:
          return format_date($value, $format);
      }
    }
  }
}
