<?php

/**
 * form callback for the main form
 * @param type $form
 * @param type $form_state
 * @param type $node
 * @return string
 */
function webform_scoring_manage_rules($form, &$form_state, $node) {

  $score_record = _webform_score_enabled($node->nid);
  $form_state['score_record'] = $score_record;
  $form_state['node'] = $node;

  $label_value = "Webform Quiz - Score";

  if ($score_record) {
    $has_score = _webform_score_has_cid($node, $score_record->cid);
    if (!$has_score) {
      $score_enabled = FALSE;
    } else {
      $score_enabled = $score_record->enabled;
      $label_value = $has_score;
    }
  } else {
    $score_enabled = FALSE;
  }

  $components = _webform_score_allowed_components($node);

  if ($components) {
    $form['new'] = array(
      '#type' => 'fieldset',
      '#title' => t("New rule"),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      'cid' => array(
        '#type' => 'select',
        '#title' => 'Component',
        '#options' => _webform_score_allowed_components($node),
      ),
      'value_score_pair' => array(
        '#type' => 'textarea',
        '#title' => t('Data'),
        '#default_value' => 'answer_a|10',
      ),
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Add'),
        '#submit' => array('webform_scoring_manage_rules_submit_add'),
      )
    );
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $label_value,
  );

  $form['actions'] = array(
    '#weight' => 10,
    '#type' => 'actions',
  );

  if ($score_enabled) {
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Update'),
    );
    $form['actions']['disable'] = array(
      '#type' => 'submit',
      '#value' => t('Disable'),
      '#submit' => array('webform_scoring_manage_rules_submit_disable'),
    );
  } else {
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Enable'),
    );
  }


  $components = _webform_score_used_components($node);
  $component_table_body = array();
  $count = 0;
  $options = array('query' => drupal_get_destination());
  foreach ($components as $item) {

    $p_item['name'] = $node->webform['components'][$item->cid]['name'];
    $p_item['type'] = $node->webform['components'][$item->cid]['type'];
    $p_item['value_score_pair'] = check_plain($item->value_score_pair);
    $p_item['actions'] = implode('<br>', array(
      l(t('edit'), "node/{$node->nid}/webform/scoring-rules/edit/{$item->id}", $options),
      l(t('delete'), "node/{$node->nid}/webform/scoring-rules/delete/{$item->id}", $options),
    ));

    $component_table_body[] = $p_item;
  }

  $headers = array("Component", "Type", "Value|Score", "Actions");
  $form['items'] = array(
    '#weight' => 100,
    '#theme' => 'table',
    '#header' => $headers,
    '#rows' => $component_table_body,
    '#attributes' => array('class' => array('mytable'))
  );


  return $form;
}

/**
 * On enable/disable
 * @param type $form
 * @param array $form_state
 */
function webform_scoring_manage_rules_submit($form, &$form_state) {
  $score_record = $form_state['score_record'];
  $node = $form_state['node'];

  if (!$score_record) {
    // create if needed
    $cid = create_a_new_hidden_component($node, $form_state['values']['label']);
    $result = db_insert('webform_scoring_enabled_webforms')
      ->fields(array('enabled' => 1, 'cid' => $cid, 'nid' => $node->nid))
      ->execute();
  } else {
    // check if the cid is still in place
    $score_record->cid = create_a_new_hidden_component($node, $form_state['values']['label'], $score_record->cid);

    // update record
    $result = db_update('webform_scoring_enabled_webforms')
      ->fields(array(
        'enabled' => 1,
        'cid' => $score_record->cid,
      ))
      ->condition('id', $score_record->id)
      ->execute();
  }
}

/**
 * On disable scoring for the selected node
 * @param type $form
 * @param type $form_state
 */
function webform_scoring_manage_rules_submit_disable($form, &$form_state) {
  $score_record = $form_state['score_record'];
  $node = $form_state['node'];

  // update record
  $result = db_update('webform_scoring_enabled_webforms')
    ->fields(array(
      'enabled' => 0,
      'cid' => 0,
    ))
    ->condition('id', $score_record->id)
    ->execute();
  
  // delete the component
  delete_custom_hidden_component($node->nid, $score_record->cid);
}

/**
 * On submit a new rule
 * @param type $form
 * @param type $form_state
 */
function webform_scoring_manage_rules_submit_add($form, &$form_state) {
  // attache to this one:
  $node = $form_state['node'];

  $result = db_insert('webform_scoring_values_scores_pair')
    ->fields(array(
      'nid' => $node->nid,
      'cid' => $form_state['values']['cid'],
      'value_score_pair' => $form_state['values']['value_score_pair'],
    ))
    ->execute();
}

/**
 * Delete a specific component at the node
 * 
 * @param type $nid
 * @param type $cid
 */
function delete_custom_hidden_component($nid, $cid = 9999) {
  $node = node_load($nid);
  #dsm($node);

  unset($node->webform['components'][$cid]);
  node_save($node);
}

/**
 * Create a hidden custom component to store the result
 * @param type $nid
 * @param type $new_component
 * @param type $cid
 * @param type $pid
 * @return type
 */
function create_a_new_hidden_component($node, $label, $cid = NULL) {

  $old_label = _webform_score_has_cid($node, $cid);
  if ($old_label) {
    // update
    $node->webform['components'][$cid]['name'] = $label;
  } else {
    // insert
    $cid = max(array_keys($node->webform['components'])) + 1;
    $safe_label = drupal_html_class($label);
    $processed_name = str_replace('-', '_', strtolower($safe_label));
    // Create the webform components array. Not sure if we need all these
    // values, but let's be sure.
    $component = array(
      'cid' => $cid,
      'pid' => 0, #(int)$pid,
      'nid' => (int) $node->nid,
      // I don't trust the admin to make a key based on input :)
      'form_key' => $processed_name,
      'name' => $label,
      // I want all lines to be numeric type component.
      'type' => 'hidden',
      'value' => '0',
      'extra' => array(),
      'mandatory' => '0',
      'weight' => -1,
      'page_num' => 1,
    );
    $node->webform['components'][$cid] = $component;
  }

  node_save($node);
  return $cid;
}
