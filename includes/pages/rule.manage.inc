<?php

/**
 * Page callback - choose which form to render
 * @todo create the delete rule callback
 * @todo create the edit rule callback
 * 
 * @param type $node
 * @param type $action
 * @param type $sid
 * @return type
 */
function webform_scoring_scoring_rules($node, $action = NULL, $rid = NULL) {
  switch ($action) {
    case 'delete':
      module_load_include('inc', 'webform_scoring', 'includes/forms/rule.delete');
      return drupal_get_form('webform_scoring_delete_rule', $node, $rid);
    case 'edit':
      module_load_include('inc', 'webform_scoring', 'includes/forms/rule.edit');
      return drupal_get_form('webform_scoring_edit_rule', $node, $rid);
    default:
      module_load_include('inc', 'webform_scoring', 'includes/forms/rule.view');
      return drupal_get_form('webform_scoring_manage_rules', $node);
  }
}

/**
 * Get a list of all used components
 * @param type $node
 * @return type
 */
function _webform_score_used_components($node) {
  $components = &drupal_static(__FUNCTION__ . '_' . $node->nid);

  if (!$components) {
    $components = array();
    $result = db_select('webform_scoring_values_scores_pair', 'a')
      ->fields('a')
      ->condition('a.nid', $node->nid)
      ->execute();
    foreach ($result as $component) {
      $components[$component->cid] = $component;
    }
  }
  return $components;
}

/**
 * Get the settings for the current webform
 * @param type $nid
 * @return type
 */
function _webform_score_enabled($nid) {
  $enabled = &drupal_static(__FUNCTION__ . '_' . $nid);

  if (empty($enabled['loaded'])) {
    $enabled['loaded'] = TRUE;
    $query = db_select('webform_scoring_enabled_webforms', 'a');
    $query->fields('a');
    $query->condition('a.nid', $nid);
    $enabled['result'] = $query->execute()->fetch();
  }

  return $enabled['result'];
}

/**
 * Load a single rule
 * @param type $rid
 * @return type
 */
function _webform_score_get_rule($node, $rid) {
  $rule = &drupal_static(__FUNCTION__ . '_' . $rid);
  if (!$rule) {
    $query = db_select('webform_scoring_values_scores_pair', 'p');
    $query->fields('p');
    $query->condition('id', $rid);
    $query->condition('nid', $node->nid);
    $rule = $query->execute()->fetch();
  }
  
  return $rule;
}

/**
 * Get a list of all possible components to create a rule for
 * 
 * @param type $node
 * @return type
 */
function _webform_score_allowed_components($node) {
  $components = &drupal_static(__FUNCTION__ . '_' . $node->nid);

  if (!$components) {
    $used = _webform_score_used_components($node);
    foreach ($node->webform['components'] as $cid => $component) {
      if (!empty($used[$cid])) {
        continue;
      }

      switch ($component['type']) {
        case 'markup':
        case 'hidden':
        case 'pagebreak':
          continue 2;
        default:
          break;
      }
      $components[$cid] = $component['name'];
    }
  }

  return $components;
}

/**
 * Check if the webform contains a certain component
 * @param type $node
 * @param type $cid
 * @return type
 */
function _webform_score_has_cid($node, $cid) {
  if (empty($node->webform)) {
    return FALSE;
  }

  if (empty($node->webform['components'][$cid]['name'])) {
    return FALSE;
  }

  return $node->webform['components'][$cid]['name'];
}
