<?php
// $Id$

function votingapi_views_tables() {
  $vote_tags = _votingapi_views_values($field = 'tag', $table = 'vote');
  $vote_types = _votingapi_views_values($field = 'value_type', $table = 'vote');

  foreach ($vote_tags as $vote_tag) {
    foreach ($vote_types as $vote_type) {
      $table = array(
        "name" => "votingapi_vote", 
        "provider" => "votingapi",
        "join" => array(
          "left" => array(
            "table" => "node",
            "field" => "nid"
          ), 
          "right" => array(
            "field" => "content_id"
          ),
          "extra" => array(
            'content_type' => 'node',
            'value_type' => $vote_type,
            'tag' => $vote_tag,
          ),
        ),
        "fields" => array(
          "value" => array(
            'name' => t("VotingAPI !type !tag value", array('!tag' => $vote_tag, '!type' => $vote_type)),
            'sortable' => true,
          ),
          "timestamp" => array(
            'name' => t("VotingAPI !type !tag timestamp", array('!tag' => $vote_tag, '!type' => $vote_type)),
            'sortable' => true,
            'handler' => 
              array(
                "views_handler_field_date_small" => t('As Short Date'), 
                "views_handler_field_date" => t('As Medium Date'), 
                "views_handler_field_date_large" => t('As Long Date'), 
                "views_handler_field_since" =>  t('As Time Ago')
              ),
            'sortable' => true,
            'help' => "Display the time the vote was cast.",
          ),
          "uid" => array(
            'name' => t("VotingAPI !type !tag user", array('!tag' => $vote_tag, '!type' => $vote_type)),
            'handler' => 'votingapi_views_handler_field_username', 
            'sortable' => true, 
          ),
        ),  
        "sorts" => array(
          "value" => array('name' => t("VotingAPI !type !tag vote", array('!tag' => $vote_tag, '!type' => $vote_type))),
          "uid" => array('name' => t("VotingAPI !type !tag voter", array('!tag' => $vote_tag, '!type' => $vote_type))),
          "timestamp" => array('name' => t("VotingAPI !type !tag timestamp", array('!tag' => $vote_tag, '!type' => $vote_type))),
        ),
        'filters' => array(
          'value' => array(
            'name' => t("VotingAPI !type !tag value", array('!tag' => $vote_tag, '!type' => $vote_type)),
            'operator' => 'views_handler_operator_gtlt',
            'handler' => 'votingapi_handler_filter_value',
            'value-type' => 'string',
            'help' => t('Filter nodes by values of the individual votes users cast for them.'),
          ),
          'timestamp' => array(
            'name' => t("VotingAPI !type !tag timestamp", array('!tag' => $vote_tag, '!type' => $vote_type)),
            'operator' => 'views_handler_operator_gtlt',
            'value' => views_handler_filter_date_value_form(),
            'handler' => 'views_handler_filter_timestamp',
            'option' => 'string',
            'help' => t('Filter nodes by the date they were voted on.')
              .' '. views_t_strings('filter date'),
          ),
          'uid' => array(
            'name' => t("VotingAPI !type !tag user", array('!tag' => $vote_tag, '!type' => $vote_type)),
            'operator' => array('IS NOT NULL' => t('Has been voted on by'), 'IS NULL' => t('Has not been voted on by')),
            'list' => 'views_handler_filter_usercurrent',
            'list-type' => 'select',
            'handler' => 'votingapi_views_handler_filter_uid_voted',
            'help' => t('Filter nodes by whether the currently logged in user has voted.'),
          )
        ),
      );
      $tables['votingapi_vote_' . $vote_tag .'_'. $vote_type] = $table;
    }
  }

  $cache_tags = _votingapi_views_values($field = 'tag', $table = 'cache');
  $cache_types = _votingapi_views_values($field = 'value_type', $table = 'cache');
  $cache_functions = _votingapi_views_values($field = 'function', $table = 'cache');

  foreach ($cache_functions as $cache_function) {
    foreach ($cache_tags as $cache_tag) {
      foreach ($cache_types as $cache_type) {
        $table = array(
          "name" => "votingapi_cache", 
          "provider" => "votingapi",
          "join" => array(
            "left" => array(
              "table" => "node",
              "field" => "nid"
            ), 
            "right" => array(
              "field" => "content_id"
            ),
            "extra" => array(
              'content_type' => 'node',
              'value_type' => $cache_type,
              'tag' => $cache_tag,
              'function' => $cache_function,
            ),
          ),
          "fields" => array(
            "value" => array(
              'name' => t("VotingAPI !type !tag result (!function)", array('!tag' => $cache_tag, '!type' => $cache_type, '!function' => $cache_function)),
              'sortable' => true,
            ),
          ),  
          "sorts" => array(
            "value" => array('name' => t("VotingAPI !type !tag result (!function)", array('!tag' => $cache_tag, '!type' => $cache_type, '!function' => $cache_function))),
          ),
          'filters' => array(
            'value' => array(
              'name' => t("VotingAPI !type !tag result (!function)", array('!tag' => $cache_tag, '!type' => $cache_type, '!function' => $cache_function)),
              'operator' => 'views_handler_operator_gtlt',
              'handler' => 'votingapi_handler_filter_value',
              'value-type' => 'string',
              'help' => t('Filter nodes by the aggregate results of votes cast.'),
            ),
          ),
        );
        $tables[implode('_', array('votingapi_cache', $cache_tag, $cache_type, $cache_function))] = $table;
      }
    }
  }

  return $tables;
}

function votingapi_views_handler_field_username($fieldinfo, $fielddata, $value, $data) {
  if ($value === NULL) {
    return '';
  }
  else {
    $user = user_load(array('uid' => $value));
    return theme('username', $user);
  }
}

function votingapi_views_handler_filter_uid_voted($op, $filter, $filterinfo, &$query) {
  $value = $filter['value'];
  $table = $filterinfo['table'];
  $column = $filterinfo['field'];
  $field = "$table.$column";

  $table_data = _views_get_tables();
  $joininfo = $table_data[$table]['join'];
  $joininfo['extra'] = array('uid' => $filter['value']);

  $query->ensure_table($table);
  $query->set_distinct(); // once you do this, there's just one node.
  $query->add_where("%s %s", $field, $filter['operator']);
}


// Both of these functions are stubbed out -- we need provisions for modules to expose
// their own formatters, to be selected by the users when setting up the view.
function votingapi_views_default_result_handler($fieldinfo, $fielddata, $value, $data) {
  return $value;
}

function votingapi_views_default_vote_handler($fieldinfo, $fielddata, $value, $data) {
  return $value;
}

function votingapi_handler_filter_value($op, $filter, $filterinfo, &$query) {
  $value = $filter['value'];
  $table = $filterinfo['table'];
  $column = $filterinfo['field'];
  $field = "$table.$column";
  $query->ensure_table($table);
  $query->add_where("%s %s %d", $field, $filter['operator'], $value);
}

/**
 * The following three functions can be used by external modules, themes,
 * etc. to display formatted results for a vote object or a vote result
 * object. It's up to a VotingAPI module to catch the 'format' $op, and
 * return something that makes sense. The first module that responds 'wins',
 * so try to filter your responses sensibly. If it's not a voting object
 * your module would normally create, just return;. The next module can
 * take a stab.
 */

function votingapi_format_tag($vobj) {
  $vals = votingapi_invoke('format', $vobj, 'tag');
  if (isset($vals[0])) {
    return $vals[0];
  }
  else {
    return ucfirst($vobj->tag);
  }
}

function votingapi_format_value($vobj) {
  $vals = votingapi_invoke('format', $vobj, 'value');
  if (isset($vals[0])) {
    return $vals[0];
  }
  else {
    return $vobj->value;
  }
}

function votingapi_format_result($vobj) {
  $vals = votingapi_invoke('format', $vobj, 'result');
  if (isset($vals[0])) {
    return $vals[0];
  }
  else {
    return $vobj->value;
  }
}
 
function _votingapi_views_values($field = 'tag', $table = 'vote') {
  static $cached;
  if (!isset($cached[$table][$field])) {
    $results = db_query("SELECT DISTINCT %s FROM {votingapi_%s} WHERE content_type = 'node'", $field, $table);
    while($result = db_fetch_object($results)) {
      $cached[$table][$field][$result->$field] = $result->$field;
    }
  }
  return $cached[$table][$field];
}