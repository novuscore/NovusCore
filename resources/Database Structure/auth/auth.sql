CREATE TABLE IF NOT EXISTS `accounts` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(64) NOT NULL,
  `salt` varchar(8) NOT NULL,
  `verifier` varchar(512) NOT NULL,
  `key` varchar(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

DELETE FROM `accounts`;
INSERT INTO `accounts` (`id`, `username`, `salt`, `verifier`, `key`) VALUES
	(1, 'loadbalancer', '2c645642', '02b0e1d33988a85a8664a0706ab36a38cb452caf6a0e20cbefc32adff9bfe8d053c16deea3ef1573852a95ed27c9207f1a98e957ca8912a9e73d17556202ad5be35156c40e6f8bf2334681b8d72eaee499ca5666ad33a01600b847e9989c6088979019db4a1c41301778001d2f9bc1be4bd86f4a07777b3d1142ff385abb857cfeba2037fa300560293a2bbf5b34fbfb243fdd45a07d47c4c3ea7247cd383759a2d82257a4fec5c1b5f27503cceed92cf9e033421f6af54ab43e1aaf6e911ad42f93b7bc8688ef7c5ca129a7bc66044aa0ea515f5fbfc2a6f8663a3099bc2e494532a60bc5c6831b0d34f4afb7d55554dab0647609b20d1df181c401646a9b61', ''),
	(2, 'region', '51421ef8', 'a10464517ff7e0e985f8c3ca0338f773325aa415215c53aafdfcd3b325102191d21867b54182f7ef021620fcce238aa6f8b3a276a93d96c85a582edb0012521ba67bbeb61bffff7941dc64aac1ec99133cfa4571ae1efc560c848b82531acb79303fcd870cfef387345a12f24bfd8688412f53d4b789a0d26504ba890d95aad44681e9e480f8624df00fd08fe6daf0245932df5310d2c57310e6c7d7bb835e6cfb723d8d779f7c8f3ab51d471face56e572ef7fd1114d319c36c23aa4230b6139d4784c01b8625d59a88b6bab03b66063b180d1adaeef82309082841b4431f7dbd2f9fdf0972da39a20edce23d505ef3975dd9f47df3f6ddd811658cc03ba804', ''),
	(3, 'auth', '6aba8c9c', '253027352a332ec866f34dd39dbdc93458b5a8f4935c766165c52a7e0d1c941d921b5749f387d308d1b45c95e4984e9825c53615b1112847b15964422b4a037263a5c74e5447b8aa8dc465524b0b0621db1e867026cad78be717bd0e97e41288a053d7f656e1496630d1df7940cc7781628b3e96d1f79a80563d13f61564b2d6bfc39884011f30a027252e46ed71a892340ffe7e634d850a86937395d16a1f4b091bd37294d8fddd010fe0048327df1cb62abf7151af62ffd31f49cdc85647c2c3bc98dff3cc02b1eafbc6800f587c86cc4268c5b796a65bfbbfc33245cc2d19d49d0936c08b7f406837ac92f33b5274716dba8283dd86a35384837d0eeb230c', ''),
	(4, 'test', '0a7593c6', '6fb147bae0f71d2c87bb4ca58e017004466349b5209c47272949b7a2fb1f949b7d25bb426a5d945ea564bc2170db1a18e40b00206a96c859da6ba840632da634664d6e14d766e9120482cfbab33ead6aa3147fe620698b92c8ba085a1efdf0ea0443c307bfd02ebae3da81ffb6e985e6df3515f79c99854ebde438f2af9ee2d5e3d613dc580e3a77796d2e9a6719dafaf40d9aff40aaf35388efc64acb8506357b833a343cf72986f4b6af0102ec3b7d63be670915be2a20f2b2c49fdb2ccccc40d09737ca16f740dcc75e06a6c0002af23bf10d90fcaa09d2f7ee6b12f47efb306bbb89a8058380f3ab7794b8e339ea4d315bf19afe5663b5e1841c69a4639c', '');

CREATE TABLE IF NOT EXISTS `realmlist` (
  `id` smallint NOT NULL AUTO_INCREMENT,
  `name` varchar(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `type` tinyint unsigned NOT NULL DEFAULT '0',
  `flags` int unsigned NOT NULL DEFAULT '0',
  `permissionMask` int unsigned NOT NULL DEFAULT '0',
  `maxConnections` int NOT NULL DEFAULT '-1',
  `population` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

DELETE FROM `realmlist`;
INSERT INTO `realmlist` (`id`, `name`, `type`, `flags`, `permissionMask`, `maxConnections`, `population`) VALUES
	(1, 'NovusCore', 0, 0, 0, -1, 0);