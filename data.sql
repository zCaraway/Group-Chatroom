-- MySQL dump 10.14  Distrib 5.5.60-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: db1651965
-- ------------------------------------------------------
-- Server version	5.5.60-MariaDB

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES gbk */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `message`
--

DROP TABLE IF EXISTS `message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `message` (
  `sender_id` int(11) NOT NULL DEFAULT '0',
  `receiver_id` int(11) NOT NULL DEFAULT '0',
  `time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `text` text,
  PRIMARY KEY (`sender_id`,`receiver_id`,`time`),
  KEY `receiver_id` (`receiver_id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `message`
--

LOCK TABLES `message` WRITE;
/*!40000 ALTER TABLE `message` DISABLE KEYS */;
INSERT INTO `message` VALUES (1,1,'2018-12-23 10:10:25','a'),(1,1,'2018-12-23 10:11:23','a'),(1,1,'2018-12-23 10:11:28','a'),(1,1,'2018-12-23 10:11:45','a'),(1,1,'2018-12-23 10:30:33','a\n'),(1,1,'2018-12-23 10:42:07','asd'),(1,1,'2018-12-23 11:06:14','asdf'),(1,1,'2018-12-23 11:06:19','asdf'),(1,1,'2018-12-23 11:18:43','dsaf'),(1,1,'2018-12-23 11:24:55','a'),(1,1,'2018-12-23 11:26:41','a'),(1,2,'2018-12-20 11:10:20','qwertysdfg'),(1,2,'2018-12-20 11:20:11','qwertysdfg'),(1,2,'2018-12-20 11:20:20','efdf'),(1,2,'2018-12-20 14:39:32','qwertysdfg'),(1,2,'2018-12-22 19:49:21','123'),(1,2,'2018-12-22 20:30:35','111'),(1,2,'2018-12-22 20:33:58','444'),(1,2,'2018-12-22 21:38:03','13123\nafds'),(1,2,'2018-12-22 21:39:30','123214'),(1,2,'2018-12-22 21:42:19','asdasd\n'),(1,2,'2018-12-23 01:36:59','123'),(1,2,'2018-12-23 01:37:44','asd'),(1,2,'2018-12-23 01:51:14','432'),(1,2,'2018-12-23 02:04:01','asd'),(1,2,'2018-12-23 02:08:44','123'),(1,2,'2018-12-23 08:41:13','123'),(1,2,'2018-12-23 09:38:25','123456õK8'),(1,2,'2018-12-23 09:40:44','12	'),(1,2,'2018-12-23 09:41:12','12	'),(1,2,'2018-12-23 09:41:23','12456'),(1,2,'2018-12-23 09:42:19','12456'),(1,2,'2018-12-23 10:23:18','asd'),(1,2,'2018-12-23 10:40:37','asdf\n'),(1,2,'2018-12-23 10:43:19','asd'),(1,2,'2018-12-23 11:05:46','123'),(1,2,'2018-12-23 11:05:53','123'),(1,2,'2018-12-23 11:05:55','123'),(1,2,'2018-12-23 11:05:56','123'),(1,2,'2018-12-23 11:05:57','123'),(1,2,'2018-12-23 11:05:58','123'),(1,2,'2018-12-23 11:05:59','123'),(1,2,'2018-12-23 11:06:00','123'),(1,2,'2018-12-23 11:13:24','32'),(1,3,'2018-12-20 11:10:20','qwertysdfg'),(1,3,'2018-12-20 11:20:20','qwertysdfg'),(2,1,'2018-12-23 10:27:44','sddsf'),(2,1,'2018-12-23 11:24:51','sdasf'),(2,1,'2018-12-23 11:29:31','123'),(2,1,'2018-12-23 12:17:34','asdf'),(2,1,'2018-12-23 12:28:51','asdf'),(2,1,'2018-12-23 12:38:27','hello'),(2,1,'2018-12-23 12:46:12','hello\n'),(2,1,'2018-12-23 12:46:15','hello\n'),(2,1,'2018-12-23 12:46:24','hello\n'),(2,1,'2018-12-23 12:53:29','hello'),(2,1,'2018-12-23 13:05:48','sdfdg'),(2,1,'2018-12-23 13:10:16','asdasvx'),(2,1,'2018-12-23 13:16:49','asdf'),(2,1,'2018-12-23 13:36:11','cbxb'),(2,1,'2018-12-23 13:36:34','vcx'),(2,2,'2018-12-23 11:09:49','asd'),(2,2,'2018-12-23 11:28:26','2123'),(2,2,'2018-12-23 12:17:11','asdas'),(2,2,'2018-12-23 12:27:52','asd'),(2,2,'2018-12-23 12:44:24','asf'),(2,2,'2018-12-23 12:44:54','dfds'),(2,2,'2018-12-23 12:45:52','asdas'),(2,2,'2018-12-23 12:53:17','qwer'),(2,2,'2018-12-23 13:35:58','asdasf'),(2,3,'2018-12-20 11:10:20','qwertysdfg'),(3,1,'2018-12-20 11:10:20','qwertysdfg'),(3,2,'2018-12-20 11:10:20','qwertysdfg');
/*!40000 ALTER TABLE `message` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user`
--

DROP TABLE IF EXISTS `user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user` (
  `id` int(11) NOT NULL,
  `name` varchar(30) DEFAULT NULL,
  `passwd` varchar(40) DEFAULT NULL,
  `oldnum` int(11) NOT NULL,
  `never_online` int(11) DEFAULT NULL,
  `a` int(11) NOT NULL,
  `b` int(11) NOT NULL,
  `g` int(11) NOT NULL,
  `r` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user`
--

LOCK TABLES `user` WRITE;
/*!40000 ALTER TABLE `user` DISABLE KEYS */;
INSERT INTO `user` VALUES (1,'pqy','e10adc3949ba59abbe56e057f20f883e',100,0,205,0,0,255),(2,'p','e10adc3949ba59abbe56e057f20f883e',6,0,0,0,0,0),(3,'p1wer45t5tt5t5','827ccb0eea8a706c4c34a16891f84e7b',100,1,0,0,0,0),(4,'p2','e10adc3949ba59abbe56e057f20f883e',100,1,0,0,0,0),(5,'p3','e10adc3949ba59abbe56e057f20f883e',100,1,0,0,0,0);
/*!40000 ALTER TABLE `user` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-12-23 14:15:31
