-- --------------------------------------------------------
--
-- brief:       sql script of camera-guard frontent
-- author:      wite_chen
-- date:        2020/01/11
--
-- --------------------------------------------------------

CREATE DATABASE IF NOT EXISTS `camera_guard_history` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `camera_guard_history`;


-- library
CREATE TABLE IF NOT EXISTS `hist_recognize_record` (
    `id` BIGINT AUTO_INCREMENT,
	`task_id` INT(11) DEFAULT 0,
    `task_name` VARCHAR(128) NOT NULL DEFAULT '' COMMENT 'name of task',
	`camera_id` BIGINT DEFAULT 0,
    `camera_name` VARCHAR(128) NOT NULL COMMENT 'name of camera',
    `person_id` BIGINT DEFAULT 0,
    `person_name` VARCHAR(128) NOT NULL COMMENT 'name of person',
    `sex` TINYINT DEFAULT 0 COMMENT '0: unknow, 1: male, 2: femal',
    `card_no` VARCHAR(128) NOT NULL DEFAULT '' COMMENT 'identity of person',
    `category` TINYINT DEFAULT 0 COMMENT 'category of person, 0: 白名单, 1: 黑名单',
    `lib_id` INT(10) default 0 COMMENT 'lib-id of person',
    `lib_name` VARCHAR(128) NOT NULL COMMENT 'name of library',
    `classify` TINYINT DEFAULT 0 COMMENT '0: stranger',
    `pic_path` VARCHAR(256) NOT NULL DEFAULT '',
    `capture_path` VARCHAR(256) NOT NULL DEFAULT '',
    `similarity` float DEFAULT 0.0,
    `remark` VARCHAR(512) NOT NULL DEFAULT '',
    `visible` TINYINT DEFAULT 1 COMMENT 'state of library, 0: valid, 1: invalid',
	`capture_time` datetime DEFAULT NULL,
    `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
	KEY `index_task_id` (`task_id`),
	KEY `index_camera_id` (`camera_id`),
    KEY `index_lib_id` (`lib_id`),
	KEY `index_card_no` (card_no),
    KEY `index_classify` (`classify`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
ALTER TABLE `hist_recognize_record` ADD COLUMN IF NOT EXISTS `task_id` INT(11) NOT NULL DEFAULT 0;
ALTER TABLE `hist_recognize_record` ADD COLUMN IF NOT EXISTS `task_name` VARCHAR(128) DEFAULT '';
ALTER TABLE `hist_recognize_record` ADD COLUMN IF NOT EXISTS `camera_id` INT(11) NOT NULL DEFAULT 0;
ALTER TABLE `hist_recognize_record` ADD COLUMN IF NOT EXISTS `camera_name` VARCHAR(128) DEFAULT '';
ALTER TABLE `hist_recognize_record` CHANGE COLUMN IF EXISTS `name` `person_name` VARCHAR(128) DEFAULT '';
ALTER TABLE `hist_recognize_record` ADD COLUMN IF NOT EXISTS `person_name` VARCHAR(128) NOT NULL DEFAULT '';
ALTER TABLE `hist_recognize_record` ADD COLUMN IF NOT EXISTS `pic_path` VARCHAR(256) NOT NULL DEFAULT '';
ALTER TABLE `hist_recognize_record` ADD COLUMN IF NOT EXISTS `capture_path` VARCHAR(256) NOT NULL DEFAULT '';
ALTER TABLE `hist_recognize_record` ADD COLUMN IF NOT EXISTS `capture_time` datetime DEFAULT NULL;
ALTER TABLE `hist_recognize_record` ADD INDEX IF NOT EXISTS `index_task_id`(task_id);
ALTER TABLE `hist_recognize_record` ADD INDEX IF NOT EXISTS `index_camera_id`(camera_id);
ALTER TABLE `hist_recognize_record` ADD INDEX IF NOT EXISTS `index_card_no`(card_no);



