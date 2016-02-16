<?php

/**
 * Form callback to edit a rule
 * 
 * @param type $form
 * @param array $form_state
 * @param type $node
 * @param type $rid
 * @return boolean
 */
function webform_scoring_edit_rule($form, &$form_state, $node, $rid) {

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#submit' => array('webform_scoring_edit_rule_save'),
    ),
    'cancel' => array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
    ),
  );

  $rule = _webform_score_get_rule($node, $rid);
  $component = NULL;

  if ($rule && $node->webform['components'][$rule->cid]) {
    $component = $node->webform['components'][$rule->cid];
  }

  $form_state['rid'] = $rule->id;

  if (!$rule || !$component) {
    drupal_set_message(t('Component not found'), 'error', FALSE);
    $form['actions']['submit']['#access'] = FALSE;
  } else {
    $form['info'] = array(
      '#markup' => t('Score map for %component', array('%component' => $component['name'])),
    );
    if ($component['type'] == 'select') {
      $form['info']['#suffix'] = '<pre>' . $component['extra']['items'] . '</pre>';
    }
    $form['value_score_pair'] = array(
      '#type' => 'textarea',
      '#default_value' => $rule->value_score_pair,
      '#required' => TRUE,
      '#title' => t('Data'),
    );
  }

  return $form;
}

function webform_scoring_edit_rule_save($form, &$form_state) {
  db_update('webform_scoring_values_scores_pair')
    ->fields(array(
      'value_score_pair' => $form_state['values']['value_score_pair'],
    ))
    ->condition('id', $form_state['rid'])
    ->execute();
}
