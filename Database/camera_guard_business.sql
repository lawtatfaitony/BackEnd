-- --------------------------------------------------------
--
-- brief:       sql script of camera-guard frontent
-- author:      wite_chen
-- date:        2020/01/11
--
-- --------------------------------------------------------



CREATE DATABASE IF NOT EXISTS `camera_guard_business` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `camera_guard_business`;


-- user
CREATE TABLE IF NOT EXISTS `ft_user` (
    `id` int(10) AUTO_INCREMENT,
    `name` varchar(128) NOT NULL COMMENT 'name of user',
    `password` varchar(128) NOT NULL COMMENT 'password of user, md5 encode',
    `remark` varchar(512) NOT NULL DEFAULT '',
    `visible` tinyint DEFAULT 1 COMMENT 'state of user, 0: valid, 1: invalid',
    `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `index_name` (`name`),
    KEY `index_visible` (`visible`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


-- person
CREATE TABLE IF NOT EXISTS `ft_person` (
    `id` BIGINT AUTO_INCREMENT,
    `lib_id` int(10) NOT NULL default 0 COMMENT 'lib-id of person',
    `name` varchar(128) NOT NULL COMMENT 'name of person',
	`pinyin` varchar(128) NOT NULL COMMENT 'pinyin of person’s name',
    `sex` tinyint DEFAULT 0 COMMENT '0: unknow, 1: male, 2: femal',
    `card_no` varchar(128) NOT NULL DEFAULT '' COMMENT 'identity of person',
    `phone` varchar(15) NOT NULL DEFAULT '' COMMENT 'phone number',
    `category` tinyint NOT NULL DEFAULT 0 COMMENT 'category of person, 0: 白名单, 1: 黑名单',
    `remark` varchar(512) NOT NULL DEFAULT '',
    `visible` tinyint DEFAULT 1 COMMENT 'state of person, 0: valid, 1: invalid',
    `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `index_lib_id` (`lib_id`),
    KEY `index_category` (`category`),
    KEY `index_visible` (`visible`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
ALTER TABLE `ft_person` ADD COLUMN IF NOT EXISTS `pinyin` varchar(128) NOT NULL COMMENT 'pinyin of person’s name';


-- picture
CREATE TABLE IF NOT EXISTS `ft_picture` (
    `id` BIGINT AUTO_INCREMENT,
    `pic_id` BIGINT DEFAULT 0 COMMENT 'id of picture',
    `person_id` BIGINT DEFAULT 0,
    `pic_url` varchar(256) NOT NULL DEFAULT '' COMMENT 'url of picture',
    `visible` tinyint DEFAULT 1 COMMENT 'state of person, 0: valid, 1: invalid',
    `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `index_visible` (`visible`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
ALTER TABLE `ft_picture` ADD COLUMN  IF NOT EXISTS `pic_id` BIGINT DEFAULT 0 COMMENT 'id of picture';


-- library
CREATE TABLE IF NOT EXISTS `ft_library` (
    `id` int(10) AUTO_INCREMENT,
    `lib_id` int(10) NOT NULL COMMENT 'lib-id of library',
    `name` varchar(128) NOT NULL COMMENT 'name of library',
    `person_count` int(10) default 0 COMMENT 'lib-id of person',
    `type` tinyint DEFAULT 0 COMMENT '0:common, 1:VIP, ...',
    `remark` varchar(512) NOT NULL DEFAULT '',
    `visible` tinyint DEFAULT 1 COMMENT 'state of library, 0: valid, 1: invalid',
    `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `index_lib_id` (`lib_id`),
    KEY `index_name` (`name`),
    KEY `index_type` (`type`),
    KEY `index_visible` (`visible`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
ALTER TABLE `ft_library` ADD COLUMN  IF NOT EXISTS `lib_id` int NOT NULL COMMENT 'lib-id of library';


-- camera
CREATE TABLE IF NOT EXISTS `ft_camera` (
    `id` int(10) AUTO_INCREMENT,
    `name` varchar(128) NOT NULL COMMENT 'name of camera',
    `rtsp` varchar(256) NOT NULL DEFAULT '',
    `ip` varchar(32) NOT NULL DEFAULT '',
    `username` varchar(128) NOT NULL DEFAULT '',
    `password` varchar(128) NOT NULL DEFAULT '',
    `type` tinyint DEFAULT 0 COMMENT '0:camera, ...',
    `remark` varchar(512) NOT NULL DEFAULT '',
    `visible` tinyint DEFAULT 1 COMMENT 'state of camera, 0: valid, 1: invalid',
    `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `index_name` (`name`),
    KEY `index_type` (`type`),
    KEY `index_visible` (`visible`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
ALTER TABLE `ft_camera` ADD COLUMN  IF NOT EXISTS `ip` varchar(32) NOT NULL DEFAULT '';
ALTER TABLE `ft_camera` ADD COLUMN  IF NOT EXISTS `username` varchar(128) NOT NULL DEFAULT '';
ALTER TABLE `ft_camera` ADD COLUMN  IF NOT EXISTS `password` varchar(128) NOT NULL DEFAULT '';

-- task
CREATE TABLE IF NOT EXISTS `ft_task` (
    `id` int(10) AUTO_INCREMENT,
    `name` varchar(128) NOT NULL COMMENT 'name of task',
    `type` tinyint DEFAULT 0 COMMENT '0: common,...',
    `camera_list1` varchar(1024) NOT NULL DEFAULT '' COMMENT 'camera list of entrance',
    `camera_list2` varchar(1024) NOT NULL DEFAULT '' COMMENT 'camera list of exit',
	`lib_list` varchar(1024) NOT NULL DEFAULT '' COMMENT 'library list',
    `interval` int(10) DEFAULT 3000 COMMENT 'timer to capture a frame',
    `threshold` float DEFAULT 0.8 COMMENT 'the line to judge a person is stranger',
    `state` tinyint DEFAULT 0 COMMENT 'state of task, 0: off, 1: on',
    `plan` varchar(256) NOT NULL DEFAULT '' COMMENT 'the plan of task, timer to start/stop the task',
    `remark` varchar(512) NOT NULL DEFAULT '',
    `visible` tinyint DEFAULT 1 COMMENT 'state of task, 0: valid, 1: invalid',
    `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `index_name` (`name`),
    KEY `index_type` (`type`),
    KEY `index_state` (`state`),
    KEY `index_visible` (`visible`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
ALTER TABLE `ft_task` ADD COLUMN  IF NOT EXISTS `lib_list` varchar(1024) NOT NULL DEFAULT '';



-- system config
CREATE TABLE IF NOT EXISTS `ft_config` (
    `id` int(10) AUTO_INCREMENT,
    `name` varchar(128) NOT NULL COMMENT 'name of config',
    `config` varchar(1024) NOT NULL DEFAULT '' COMMENT 'the detail of config',
    `visible` tinyint DEFAULT 1 COMMENT 'state of task, 0: valid, 1: invalid',
    `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `index_name` (`name`),
    KEY `index_visible` (`visible`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


-- create the admin account
INSERT INTO ft_user(name,password) SELECT 'admin','0192023a7bbd73250516f069df18b500' 
    WHERE NOT EXISTS (SELECT * FROM ft_user WHERE name = 'admin');
