-- MySQL dump 10.13  Distrib 5.5.16, for Win32 (x86)
--
-- Host: 127.0.0.1    Database: novuscore
-- ------------------------------------------------------
-- Server version	5.5.16

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `account_log_object_type`
--

DROP TABLE IF EXISTS `account_log_object_type`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `account_log_object_type` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `table_name` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_log_object_type_id_uindex` (`id`),
  UNIQUE KEY `account_log_object_type_table_name_uindex` (`table_name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `account_log_object_type`
--


--
-- Table structure for table `account_log_type`
--

DROP TABLE IF EXISTS `account_log_type`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `account_log_type` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `log_type` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_log_type_id_uindex` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `account_log_type`
--


--
-- Table structure for table `account_log`
--

DROP TABLE IF EXISTS `account_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `account_log` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int(11) unsigned NOT NULL,
  `changed_by_account_id` int(11) unsigned NOT NULL,
  `log_type_id` int(11) unsigned NOT NULL,
  `description` longtext,
  `object_type_id` int(11) unsigned DEFAULT NULL,
  `object_id` int(11) unsigned DEFAULT NULL,
  `created_on` datetime NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_log_id_uindex` (`id`),
  KEY `account_log_account_id_fk` (`account_id`),
  KEY `account_log_account_id_fk_2` (`changed_by_account_id`),
  KEY `account_log_account_log_object_type_id_fk` (`object_type_id`),
  KEY `account_log_account_log_type_id_fk` (`log_type_id`),
  CONSTRAINT `account_log_account_id_fk` FOREIGN KEY (`account_id`) REFERENCES `account` (`id`),
  CONSTRAINT `account_log_account_id_fk_2` FOREIGN KEY (`changed_by_account_id`) REFERENCES `account` (`id`),
  CONSTRAINT `account_log_account_log_object_type_id_fk` FOREIGN KEY (`object_type_id`) REFERENCES `account_log_object_type` (`id`),
  CONSTRAINT `account_log_account_log_type_id_fk` FOREIGN KEY (`log_type_id`) REFERENCES `account_log_type` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `account_log`
--


--
-- Table structure for table `account`
--

DROP TABLE IF EXISTS `account`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `account` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(64) NOT NULL,
  `salt` varchar(8) NOT NULL,
  `verifier` varchar(512) NOT NULL,
  `key` varchar(32) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_id_uindex` (`id`),
  UNIQUE KEY `account_username_uindex` (`username`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `account`
--

INSERT INTO `account` VALUES (1,'loadbalancer','2c645642','02b0e1d33988a85a8664a0706ab36a38cb452caf6a0e20cbefc32adff9bfe8d053c16deea3ef1573852a95ed27c9207f1a98e957ca8912a9e73d17556202ad5be35156c40e6f8bf2334681b8d72eaee499ca5666ad33a01600b847e9989c6088979019db4a1c41301778001d2f9bc1be4bd86f4a07777b3d1142ff385abb857cfeba2037fa300560293a2bbf5b34fbfb243fdd45a07d47c4c3ea7247cd383759a2d82257a4fec5c1b5f27503cceed92cf9e033421f6af54ab43e1aaf6e911ad42f93b7bc8688ef7c5ca129a7bc66044aa0ea515f5fbfc2a6f8663a3099bc2e494532a60bc5c6831b0d34f4afb7d55554dab0647609b20d1df181c401646a9b61',''),(2,'region','51421ef8','a10464517ff7e0e985f8c3ca0338f773325aa415215c53aafdfcd3b325102191d21867b54182f7ef021620fcce238aa6f8b3a276a93d96c85a582edb0012521ba67bbeb61bffff7941dc64aac1ec99133cfa4571ae1efc560c848b82531acb79303fcd870cfef387345a12f24bfd8688412f53d4b789a0d26504ba890d95aad44681e9e480f8624df00fd08fe6daf0245932df5310d2c57310e6c7d7bb835e6cfb723d8d779f7c8f3ab51d471face56e572ef7fd1114d319c36c23aa4230b6139d4784c01b8625d59a88b6bab03b66063b180d1adaeef82309082841b4431f7dbd2f9fdf0972da39a20edce23d505ef3975dd9f47df3f6ddd811658cc03ba804',''),(3,'auth','6aba8c9c','253027352a332ec866f34dd39dbdc93458b5a8f4935c766165c52a7e0d1c941d921b5749f387d308d1b45c95e4984e9825c53615b1112847b15964422b4a037263a5c74e5447b8aa8dc465524b0b0621db1e867026cad78be717bd0e97e41288a053d7f656e1496630d1df7940cc7781628b3e96d1f79a80563d13f61564b2d6bfc39884011f30a027252e46ed71a892340ffe7e634d850a86937395d16a1f4b091bd37294d8fddd010fe0048327df1cb62abf7151af62ffd31f49cdc85647c2c3bc98dff3cc02b1eafbc6800f587c86cc4268c5b796a65bfbbfc33245cc2d19d49d0936c08b7f406837ac92f33b5274716dba8283dd86a35384837d0eeb230c',''),(4,'test','0a7593c6','6fb147bae0f71d2c87bb4ca58e017004466349b5209c47272949b7a2fb1f949b7d25bb426a5d945ea564bc2170db1a18e40b00206a96c859da6ba840632da634664d6e14d766e9120482cfbab33ead6aa3147fe620698b92c8ba085a1efdf0ea0443c307bfd02ebae3da81ffb6e985e6df3515f79c99854ebde438f2af9ee2d5e3d613dc580e3a77796d2e9a6719dafaf40d9aff40aaf35388efc64acb8506357b833a343cf72986f4b6af0102ec3b7d63be670915be2a20f2b2c49fdb2ccccc40d09737ca16f740dcc75e06a6c0002af23bf10d90fcaa09d2f7ee6b12f47efb306bbb89a8058380f3ab7794b8e339ea4d315bf19afe5663b5e1841c69a4639c','');

--
-- Table structure for table `ban_reason`
--

DROP TABLE IF EXISTS `ban_reason`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ban_reason` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `parent_id` int(11) unsigned DEFAULT NULL,
  `reason` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `ban_reason_id_uindex` (`id`),
  KEY `ban_reason_parent_id_index` (`parent_id`),
  KEY `ban_reason_id_parent_id_index` (`id`,`parent_id`),
  KEY `ban_reason_parent_id_id_index` (`parent_id`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ban_reason`
--


--
-- Table structure for table `ban`
--

DROP TABLE IF EXISTS `ban`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ban` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int(11) unsigned NOT NULL,
  `ban_reason_id` int(11) unsigned NOT NULL,
  `banned_by_account_id` int(11) unsigned NOT NULL,
  `banned_until` datetime NOT NULL,
  `comment` longtext,
  `created_on` datetime NOT NULL,
  `updated_on` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `ban_id_uindex` (`id`),
  KEY `ban_account_id_index` (`account_id`),
  KEY `ban_account_id_fk_2` (`banned_by_account_id`),
  KEY `ban_ban_reason_id_fk` (`ban_reason_id`),
  CONSTRAINT `ban_account_id_fk` FOREIGN KEY (`account_id`) REFERENCES `account` (`id`),
  CONSTRAINT `ban_account_id_fk_2` FOREIGN KEY (`banned_by_account_id`) REFERENCES `account` (`id`),
  CONSTRAINT `ban_ban_reason_id_fk` FOREIGN KEY (`ban_reason_id`) REFERENCES `ban_reason` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ban`
--


--
-- Table structure for table `ip_ban`
--

DROP TABLE IF EXISTS `ip_ban`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ip_ban` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `ip_range` varchar(255) NOT NULL,
  `ban_reason_id` int(11) unsigned NOT NULL,
  `banned_by_account_id` int(11) unsigned NOT NULL,
  `banned_until` datetime NOT NULL,
  `comment` longtext,
  `created_on` datetime NOT NULL,
  `updated_on` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `ip_ban_id_uindex` (`id`),
  KEY `ip_ban_ban_reason_id_fk` (`ban_reason_id`),
  KEY `ip_ban_account_id_fk` (`banned_by_account_id`),
  CONSTRAINT `ip_ban_account_id_fk` FOREIGN KEY (`banned_by_account_id`) REFERENCES `account` (`id`),
  CONSTRAINT `ip_ban_ban_reason_id_fk` FOREIGN KEY (`ban_reason_id`) REFERENCES `ban_reason` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ip_ban`
--


--
-- Table structure for table `rbac_account_role`
--

DROP TABLE IF EXISTS `rbac_account_role`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rbac_account_role` (
  `account_id` int(11) unsigned NOT NULL,
  `role_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`account_id`,`role_id`),
  UNIQUE KEY `rbac_account_role_pk_2` (`role_id`,`account_id`),
  UNIQUE KEY `rbac_account_role_account_id_role_id_uindex` (`account_id`,`role_id`),
  CONSTRAINT `rbac_account_role_account_id_fk` FOREIGN KEY (`account_id`) REFERENCES `account` (`id`),
  CONSTRAINT `rbac_account_role_rbac_role_id_fk` FOREIGN KEY (`role_id`) REFERENCES `rbac_role` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `rbac_account_role`
--


--
-- Table structure for table `rbac_group`
--

DROP TABLE IF EXISTS `rbac_group`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rbac_group` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `group` varchar(255) CHARACTER SET utf8 NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `rbac_right_group_id_group_uindex` (`id`,`group`),
  UNIQUE KEY `rbac_right_group_id_uindex` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `rbac_group`
--

INSERT INTO `rbac_group` VALUES (1,'Group 1'),(2,'Group 2');

--
-- Table structure for table `rbac_group_right`
--

DROP TABLE IF EXISTS `rbac_group_right`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rbac_group_right` (
  `group_id` int(11) unsigned NOT NULL,
  `right_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`group_id`,`right_id`),
  UNIQUE KEY `rbac_group_right_pk_2` (`right_id`,`group_id`),
  UNIQUE KEY `rbac_group_right_group_id_right_id_uindex` (`group_id`,`right_id`),
  CONSTRAINT `rbac_group_right_rbac_group_id_fk` FOREIGN KEY (`group_id`) REFERENCES `rbac_group` (`id`),
  CONSTRAINT `rbac_group_right_rbac_right_id_fk` FOREIGN KEY (`right_id`) REFERENCES `rbac_right` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `rbac_group_right`
--


--
-- Table structure for table `rbac_right`
--

DROP TABLE IF EXISTS `rbac_right`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rbac_right` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `right` varchar(255) CHARACTER SET utf8 NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `rbac_right_id_right_uindex` (`id`,`right`),
  UNIQUE KEY `rbac_right_id_uindex` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `rbac_right`
--

INSERT INTO `rbac_right` VALUES (1,'right 1'),(2,'right 2'),(3,'right 3'),(4,'right 4'),(5,'right 5'),(6,'right 6'),(7,'right 7'),(8,'right 8'),(9,'right 9'),(10,'right 10');

--
-- Table structure for table `rbac_role_group`
--

DROP TABLE IF EXISTS `rbac_role_group`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rbac_role_group` (
  `role_id` int(11) unsigned NOT NULL,
  `group_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`role_id`,`group_id`),
  UNIQUE KEY `rbac_role_group_pk` (`group_id`,`role_id`),
  UNIQUE KEY `rbac_role_group_role_id_group_id_uindex` (`role_id`,`group_id`),
  CONSTRAINT `rbac_role_group_rbac_group_id_fk` FOREIGN KEY (`group_id`) REFERENCES `rbac_group` (`id`),
  CONSTRAINT `rbac_role_group_rbac_role_id_fk` FOREIGN KEY (`role_id`) REFERENCES `rbac_role` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `rbac_role_group`
--


--
-- Table structure for table `rbac_role_right`
--

DROP TABLE IF EXISTS `rbac_role_right`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rbac_role_right` (
  `role_id` int(11) unsigned NOT NULL,
  `right_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`role_id`,`right_id`),
  UNIQUE KEY `rbac_role_right_right_id_role_id_uindex` (`right_id`,`role_id`),
  UNIQUE KEY `rbac_role_right_role_id_right_id_uindex` (`role_id`,`right_id`),
  CONSTRAINT `rbac_role_right_rbac_right_id_fk` FOREIGN KEY (`right_id`) REFERENCES `rbac_right` (`id`),
  CONSTRAINT `rbac_role_right_rbac_role_id_fk` FOREIGN KEY (`role_id`) REFERENCES `rbac_role` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `rbac_role_right`
--


--
-- Table structure for table `rbac_role`
--

DROP TABLE IF EXISTS `rbac_role`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rbac_role` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `role` varchar(255) CHARACTER SET utf8 NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `rbac_role_id_role_uindex` (`id`,`role`),
  UNIQUE KEY `rbac_role_id_uindex` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `rbac_role`
--


--
-- Table structure for table `realm_tag`
--

DROP TABLE IF EXISTS `realm_tag`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `realm_tag` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `tag` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `realm_tag_id_uindex` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `realm_tag`
--


--
-- Table structure for table `realmlist`
--

DROP TABLE IF EXISTS `realmlist`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `realmlist` (
  `id` smallint(6) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `permissionMask` int(10) unsigned NOT NULL DEFAULT '0',
  `maxConnections` int(11) NOT NULL DEFAULT '-1',
  `population` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `join_right_id` int(11) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `realmlist_id_uindex` (`id`),
  KEY `realmlist_rbac_right_id_fk` (`join_right_id`),
  CONSTRAINT `realmlist_rbac_right_id_fk` FOREIGN KEY (`join_right_id`) REFERENCES `rbac_right` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `realmlist`
--

INSERT INTO `realmlist` VALUES (1,'NovusCore',0,0,0,-1,0,NULL);

--
-- Table structure for table `realmlist_realm_tag`
--

DROP TABLE IF EXISTS `realmlist_realm_tag`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `realmlist_realm_tag` (
  `realm_id` smallint(6) unsigned NOT NULL,
  `tag_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`realm_id`,`tag_id`),
  UNIQUE KEY `realmlist_realm_tag_realm_id_tag_id_uindex` (`realm_id`,`tag_id`),
  UNIQUE KEY `realmlist_realm_tag_tag_id_realm_id_uindex` (`tag_id`,`realm_id`),
  CONSTRAINT `realmlist_realm_tag_realmlist_id_fk` FOREIGN KEY (`realm_id`) REFERENCES `realmlist` (`id`),
  CONSTRAINT `realmlist_realm_tag_realm_tag_id_fk` FOREIGN KEY (`tag_id`) REFERENCES `realm_tag` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `realmlist_realm_tag`
--

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;